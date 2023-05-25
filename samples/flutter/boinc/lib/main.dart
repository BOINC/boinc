import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/foundation.dart' show kIsWeb;

import 'package:flutter_svg/avd.dart';
import 'package:neat_periodic_task/neat_periodic_task.dart';
import 'package:path/path.dart' as path;
import 'package:http/http.dart' as http;
import 'package:xml/xml.dart';
import 'package:convert/convert.dart';
import 'package:crypto/crypto.dart';

import 'dart:convert';
import 'dart:async';
import 'dart:io';
import 'dart:io' if (dart.library.io)   'package:boinc/windows.dart';
import 'dart:io' if (dart.library.io)   'package:boinc/macos.dart';
import 'dart:io' if (dart.library.io)   'package:boinc/linux.dart';
import 'dart:io' if (dart.library.html) 'package:boinc/web.dart';


void main() => runApp(MyApp());


class MyApp extends StatefulWidget {
  @override
  MyAppState createState() => MyAppState();

  static MyAppState? of(BuildContext context) =>
      context.findAncestorStateOfType<MyAppState>();
}

enum Section
{
  Run,
  Connect,
  State,
  Tasks,
  Notices,
  Projects,
  Preference,
  Help,
  Report,
  Event,
}


class MyAppState extends State<MyApp> {
  static const String channel = 'edu.berkeley.boinc/client';
  static const platform = const MethodChannel(channel);
  NeatPeriodicTaskScheduler? scheduler;
  MyAppState(){
    scheduler = NeatPeriodicTaskScheduler(
      interval: Duration(seconds: 1),
      name: 'update_state',
      timeout: Duration(seconds: 1),
      task: () async => updateState(),
      minCycle: Duration(milliseconds: 500),
    );
  }

  /// 1) our themeMode "state" field
  final String appName = 'Boinc';
  ThemeMode _themeMode = ThemeMode.system;
  Section _section = Section.Preference;
  String title = "";
  int? authId;
  int authSeqno = 1;
  String authSalt = "";
  Future<String> password = () async {return "";}();
  bool isReadFile = false;
  bool isClientRunning = false;
  bool isSchedule = false;
  Iterable<XmlElement> _projects = [];
  Iterable<XmlElement> _results = [];
  Iterable<XmlElement> _workUnits = [];

  @override
  Widget build(BuildContext context) {

    title = getSectionTitle(_section);
    return MaterialApp(
      title: appName,
      theme: ThemeData(),
      darkTheme: ThemeData.dark(),
      themeMode: _themeMode, // 2) ← ← ← use "state" field here //////////////
      home: MyHomePage(title: title),
    );
  }

  String getSectionTitle(Section section) {
    return section.toString().substring(8);
  }

  void changeSection(Section section) {
    setState(() {
      _section = section;
    });
  }

  void changeState(Iterable<XmlElement> projects,
      Iterable<XmlElement> results,
      Iterable<XmlElement> workUnits) {
    setState(() {
      _projects = projects;
      _results = results;
      _workUnits = workUnits;
    });
  }

  Section getSection() {
    return _section;
  }

  /// 3) Call this to change theme from any context using "of" accessor
  /// e.g.:
  /// MyApp.of(context).changeTheme(ThemeMode.dark);
  void changeTheme(ThemeMode themeMode) {
    setState(() {
      _themeMode = themeMode;
    });
  }

  void runAndroidClient() async {
    try {
      final bool result = await platform.invokeMethod('runClient');
    } on PlatformException catch (e) {
      print('${e.message}');
    }
  }

  void runClient() {
    if (kIsWeb) {

    } else {
      final execuPath = path.dirname(Platform.resolvedExecutable);
      final boincPath = path.join(execuPath,
          "data", "flutter_assets", "assets");
      if (Platform.isAndroid) {
        runAndroidClient();
      }
      if (Platform.isLinux) {
        systemLinux("chmod +x " + path.join(boincPath, "linux", "boinc"));
        systemLinux(path.join(boincPath, "linux", "boinc") + " &");
      }
      if (Platform.isWindows) {
        systemWindows(path.join(boincPath, "windows", "boinc.exe"));
      }
      if (Platform.isMacOS) {
        systemMacOS("chmod +x " + path.join(boincPath, "macos", "boinc"));
        systemMacOS(path.join(boincPath, "macos", "boinc") + " &");
      }
    }
    isClientRunning = true;
  }

  void updateState() {
    guiRpc("get_state").then((String response) {
      print("we got get_state!!");
      var document = XmlDocument.parse(response);
      var projects = document.findAllElements("project");
      var workUnits = document.findAllElements("workunit");
      var results = document.findAllElements("result");
      print("projects: ${projects.length}");
      print("results: ${results.length}");
      print("workUnits: ${workUnits.length}");
      changeState(projects, results, workUnits);
      // print(document);
    });
  }

  Future<String> guiRpc(String request) async {
    final builder = XmlBuilder();
    builder.element('boinc_gui_rpc_request', nest: () {
      builder.element(request);
    });
    String requestXml = builder.buildDocument().toXmlString();

    var url = Uri.parse('http://localhost:31416');
    Map<String, String> headers = {};
    if (authId != null) {
      var authHash = hex.encode(md5.convert(latin1.encode("$authSeqno"+"${await password}"+authSalt+requestXml)).bytes);
      headers = {"Auth-ID": "$authId", "Auth-Seqno": "$authSeqno", "Auth-Hash": authHash};
      authSeqno++;
    }
    headers = {
      ...headers,
      "Content-Length": "${latin1.encode(requestXml).lengthInBytes}",
    };
    print("url: $url");
    print("headers: $headers");
    print("requestXml: $requestXml");
    if (kIsWeb) {
      http.Response response = await http.post(url, headers: headers, body: requestXml, encoding: latin1);
      return response.body;
    } else {
      HttpClient client = HttpClient();
      return await client.postUrl(url)
          .then((HttpClientRequest request) {
        headers.forEach((key, value) {
          request.headers.add(key, value, preserveHeaderCase: true);
        });
        request.headers.set("Content-Length", "${latin1.encode(requestXml).lengthInBytes}", preserveHeaderCase: true);
        request.headers.contentType = ContentType.parse("text/xml");
        request.write(requestXml);
        return request.close();
      }).then((HttpClientResponse response) {
        return response.transform(latin1.decoder).join();
      });
    }
  }

  Future<String> readFile(String filePath) async {
    File cfg = File(filePath);
    String content = "";
    try {
      // Read the file
      content = await cfg.readAsString();
    } catch(e) {}
    return content.trim();
  }

  Future<void> connectClient() {
    var filePath = "";
    if (kIsWeb) {

    } else {
      if (Platform.isWindows) {
        // filePath = 'C:\\ProgramData\\BOINC\\gui_rpc_auth.cfg';
      }
      if (Platform.isLinux) {

      }
      if (Platform.isMacOS) {

      }
      if (Platform.isAndroid) {

      }
    }

    if (filePath != "" && !isReadFile) {
      password = readFile(filePath);
      isReadFile = true;
    }

    return guiRpc("get_auth_id").then((String response) {
      print("we got get_auth_id!!");
      var document = XmlDocument.parse(response);
      authId = int.parse(document.findAllElements("auth_id").first.text);
      authSalt = document.findAllElements("auth_salt").first.text;
      print("authId: $authId");
      print("authSalt: $authSalt");
    });
  }
}

class Preference extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    var myAppContext = MyApp.of(context)!;
    return Scaffold(
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            Text(
              'Choose your theme:',
            ),
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                /// //////////////////////////////////////////////////////
                /// Change theme & rebuild to show it using these buttons
                ElevatedButton(
                  onPressed: () => myAppContext.changeTheme(ThemeMode.light),
                  child: Text('Light')
                ),
                ElevatedButton(
                  onPressed: () => myAppContext.changeTheme(ThemeMode.dark),
                  child: Text('Dark'),
                ),
                /// //////////////////////////////////////////////////////
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class Projects extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    var myAppContext = MyApp.of(context)!;
    Iterable<XmlElement> projects = myAppContext._projects;
    return Scaffold(
        body: ListView.builder
          (
            itemCount: projects.length,
            itemBuilder: (BuildContext context, int index) {
              var projectName = projects.elementAt(index).findElements("project_name").first.text;
              return Text("Name: $projectName");
            }
        )
    );
  }
}

class Tasks extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    var myAppContext = MyApp.of(context)!;
    Iterable<XmlElement> results = myAppContext._results;
    Iterable<XmlElement> workUnits = myAppContext._workUnits;
    print("results.length: ${results.length}");
    print("workUnits.length: ${workUnits.length}");
    print("results: $results");
    print("workUnits: $workUnits");
    return Scaffold(
        body: ListView.builder
          (
            itemCount: results.length,
            itemBuilder: (BuildContext context, int index) {
              var result = results.elementAt(index);
              String wu_name = result.findElements("wu_name").first.text;
              String appName = workUnits.firstWhere((workUnit) => workUnit.findElements("name").first.text == wu_name)
                  .findElements("app_name")
                  .first.text;
              bool active = false;
              String remain = "-1";
              try {
                active = result.findAllElements("active_task_state").first.text == "1";
                remain = result.findElements("estimated_cpu_time_remaining").first.text;
              } catch(e) {}
              return Text("Name: $appName, Active: $active, Remain: $remain");
            }
        )
    );
  }
}

class MyHomePage extends StatelessWidget {
  final String title;

  MyHomePage({Key? key, required this.title}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    var appBar = AppBar(title: Text(title), centerTitle: true);
    var textColor = Theme.of(context).textTheme.bodyText1!.color;
    var myAppContext = MyApp.of(context)!;
    Widget? body;
    var section = myAppContext.getSection();

    /// You can easily control the section for example inside the initState where you check
    /// if the user logged in, or other related logic
    switch (section)
    {
      case Section.Run:
        break;
      case Section.Connect:
        break;
      case Section.State:
        break;
      case Section.Tasks:
        body = Tasks();
        break;
      case Section.Notices:
        break;
      case Section.Projects:
        body = Projects();
        break;
      case Section.Preference:
        body = Preference();
        break;
      case Section.Help:
        break;
      case Section.Report:
        break;
      case Section.Event:
        break;
      default:
        break;
    }

    return Scaffold(
      appBar: appBar,
      body: body,
      drawer: Drawer(
        // Add a ListView to the drawer. This ensures the user can scroll
        // through the options in the drawer if there isn't enough vertical
        // space to fit everything.
        child: ListView(
          // Important: Remove any padding from the ListView.
          padding: EdgeInsets.zero,
          children: <Widget>[
            Container(
            height: appBar.preferredSize.height,
            child: DrawerHeader(
              decoration: BoxDecoration(
                color: Theme.of(context).primaryColor,
                shape: BoxShape.rectangle,
              ),
              child: Text(myAppContext.appName),
            )),
            ListTile(
              leading: SizedBox.fromSize(
                size: Size.fromRadius(12),
                child: FittedBox(
                  child: AvdPicture.asset("res/drawable/ic_boinc.xml"),
                ),
              ),
              title: Text('Run Client'),
              onTap: () {
                myAppContext.changeSection(Section.Run);
                myAppContext.runClient();

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset("res/drawable/ic_baseline_wifi.xml", color: textColor),
              title: Text('Connect Client'),
              onTap: () {
                myAppContext.changeSection(Section.Connect);

                myAppContext.connectClient().then((_) {
                  if (myAppContext.authId != null && !myAppContext.isSchedule) {
                    myAppContext.isSchedule = true;
                    myAppContext.scheduler!.start();
                  }
                });

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset("res/drawable/ic_baseline_list.xml", color: textColor),
              title: Text('Tasks'),
              onTap: () {
                myAppContext.changeSection(Section.Tasks);

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset('res/drawable/ic_baseline_email.xml', color: textColor),
              title: Text('Notices'),
              onTap: () {
                myAppContext.changeSection(Section.Notices);

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: SizedBox.fromSize(
                size: Size.fromRadius(12),
                child: FittedBox(
                  child: AvdPicture.asset("res/drawable/ic_projects.xml", color: textColor),
                ),
              ),
              title: Text('Projects'),
              onTap: () {
                myAppContext.changeSection(Section.Projects);

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset('res/drawable/ic_baseline_add_box.xml', color: textColor),
              title: Text('Add Project'),
              onTap: () {
                myAppContext.changeSection(Section.Projects);

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset('res/drawable/ic_baseline_settings.xml', color: textColor),
              title: Text('Preference'),
              onTap: () {
                myAppContext.changeSection(Section.Preference);

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset('res/drawable/ic_baseline_help.xml', color: textColor),
              title: Text('Help'),
              onTap: () {
                myAppContext.changeSection(Section.Help);

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset('res/drawable/ic_baseline_bug_report.xml', color: textColor),
              title: Text('Report Issue'),
              onTap: () {
                myAppContext.changeSection(Section.Report);

                Navigator.pop(context);
              },
            ),
            ListTile(
              leading: AvdPicture.asset('res/drawable/ic_baseline_warning.xml', color: textColor),
              title: Text('Event log'),
              onTap: () {
                myAppContext.changeSection(Section.Event);

                Navigator.pop(context);
              },
            ),
          ],
        ),
      ),
    );
  }
}


