#!/usr/bin/env python

# $Id$
# cgi/php web server

import BaseHTTPServer, CGIHTTPServer
import sys, os, urllib, select
import random, time                     # XXX

php_path = None
possible_php_paths = [ '/usr/lib/cgi-bin/php4',
                       'PROGRAM_PATH/fake_php.py' ]
def setup_php(program_path):
    global php_path
    for p in possible_php_paths:
        p = p.replace('PROGRAM_PATH', program_path)
        if os.path.exists(p):
            php_path = p
            return
    raise Exception("No php binary found - not even fake_php.py (program_path=%s) !"%program_path)

class PHPHTTPRequestHandler(CGIHTTPServer.CGIHTTPRequestHandler):
    def is_cgi(self):
        if os.path.split(self.path)[1] == '':
            index_php = os.path.join(self.path, 'index.php')
            if os.path.exists(self.translate_path(index_php)):
                self.path = index_php
        if self.path.find('.php') != -1:
            self.cgi_info = os.path.split(self.path)
            return True

        for p in self.cgi_directories:
            p = os.path.join(p,'')
            if self.path.startswith(p):
                self.cgi_info = os.path.split(self.path)
                return True
        return False

    def run_cgi(self):
        """Execute a CGI script."""
        dir, rest = self.cgi_info
        i = rest.rfind('?')
        if i >= 0:
            rest, query = rest[:i], rest[i+1:]
        else:
            query = ''
        i = rest.find('/')
        if i >= 0:
            script, rest = rest[:i], rest[i:]
        else:
            script, rest = rest, ''
        scriptname = dir + '/' + script
        is_php = script.endswith('.php')
        # print "#### cgi_info=%s,dir=%s,rest=%s,script=%s,scriptname=%s,is_php=%s"%(self.cgi_info,dir,rest,script,scriptname,is_php)
        if is_php:
            if not php_path: raise Exception('php_path not set')
            scriptfile = php_path
            sourcefile = self.translate_path(scriptname)
        else:
            scriptfile = self.translate_path(scriptname)
        if not os.path.exists(scriptfile):
            self.send_error(404, "No such CGI script (%s)" % `scriptname`)
            return
        if not os.path.isfile(scriptfile):
            self.send_error(403, "CGI script is not a plain file (%s)" %
                            `scriptname`)
            return
        ispy = self.is_python(scriptname)
        if not ispy:
            if not (self.have_fork or self.have_popen2 or self.have_popen3):
                self.send_error(403, "CGI script is not a Python script (%s)" %
                                `scriptname`)
                return
            if not self.is_executable(scriptfile):
                self.send_error(403, "CGI script is not executable (%s)" %
                                `scriptname`)
                return

        # Reference: http://hoohoo.ncsa.uiuc.edu/cgi/env.html
        # XXX Much of the following could be prepared ahead of time!
        env = {}
        env['DOCUMENT_ROOT'] = os.getcwd()
        env['SERVER_SOFTWARE'] = self.version_string()
        env['SERVER_NAME'] = self.server.server_name
        env['GATEWAY_INTERFACE'] = 'CGI/1.1'
        env['SERVER_PROTOCOL'] = self.protocol_version
        env['SERVER_PORT'] = str(self.server.server_port)
        env['REQUEST_METHOD'] = self.command
        uqrest = urllib.unquote(self.cgi_info[1])
        env['REQUEST_URI'] = self.path
        # env['PATH_INFO'] = uqrest
        # env['PATH_TRANSLATED'] = self.translate_path(uqrest)
        env['SCRIPT_NAME'] = scriptname
        env['SCRIPT_FILENAME'] = self.translate_path(scriptname)
        if query:
            env['QUERY_STRING'] = query
        host = self.address_string()
        if host != self.client_address[0]:
            env['REMOTE_HOST'] = host
        env['REMOTE_ADDR'] = self.client_address[0]
        env['REDIRECT_STATUS'] = '1'      # for php
        # XXX AUTH_TYPE
        # XXX REMOTE_USER
        # XXX REMOTE_IDENT
        if self.headers.typeheader is None:
            env['CONTENT_TYPE'] = self.headers.type
        else:
            env['CONTENT_TYPE'] = self.headers.typeheader
        length = self.headers.getheader('content-length')
        if length:
            env['CONTENT_LENGTH'] = length
        accept = []
        for line in self.headers.getallmatchingheaders('accept'):
            if line[:1] in "\t\n\r ":
                accept.append(line.strip())
            else:
                accept = accept + line[7:].split(',')
        env['HTTP_ACCEPT'] = ','.join(accept)
        ua = self.headers.getheader('user-agent')
        if ua:
            env['HTTP_USER_AGENT'] = ua
        co = filter(None, self.headers.getheaders('cookie'))
        if co:
            env['HTTP_COOKIE'] = ', '.join(co)
        # XXX Other HTTP_* headers
        if not self.have_fork:
            # Since we're setting the env in the parent, provide empty
            # values to override previously set values
            for k in ('QUERY_STRING', 'REMOTE_HOST', 'CONTENT_LENGTH',
                      'HTTP_USER_AGENT', 'HTTP_COOKIE'):
                env.setdefault(k, "")
        os.environ.update(env)

        self.send_response(200, "Script output follows")

        decoded_query = query.replace('+', ' ')

        if self.have_fork:
            # Unix -- fork as we should
            if is_php:
                args = [php_path, sourcefile]
            else:
                args = [script]
            if '=' not in decoded_query:
                args.append(decoded_query)
            self.wfile.flush() # Always flush before forking
            pid = os.fork()
            if pid != 0:
                # Parent
                pid, sts = os.waitpid(pid, 0)
                # throw away additional data [see bug #427345]
                while select.select([self.rfile], [], [], 0)[0]:
                    try:
                        if not self.rfile.read(1):
                            break
                    except:
                        break
                if sts:
                    self.log_error("CGI script exit status %#x", sts)
                return
            # Child
            try:
                if 0:
                    time.sleep(.1)
                    fn = '/tmp/a%d'%random.randint(1000,10000)
                    f = open(fn, 'w')
                    s = ''
                    while select.select([self.rfile], [], [], 0)[0]:
                        try:
                            c = self.rfile.read(1)
                            if not c:
                                break
                            s += c
                        except:
                            break
                    print '### input:', repr(s)
                    print >>f, s
                    f.close()
                    self.rfile = open(fn, 'r')
                os.dup2(self.rfile.fileno(), 0)
                os.dup2(self.wfile.fileno(), 1)
                os.chdir(self.translate_path(dir)) # KC
                os.execve(scriptfile, args, os.environ)
            except:
                self.server.handle_error(self.request, self.client_address)
                os._exit(127)

        else:
            raise SystemExit('need fork()')

def serve(bind='localhost', port=8000, handler=PHPHTTPRequestHandler):
    httpd = BaseHTTPServer.HTTPServer((bind,port), handler)
    httpd.serve_forever()

if __name__ == '__main__':
    setup_php(os.path.realpath(os.path.dirname(sys.argv[0])))
    serve()
