import sys
import s3

def help():
    print('Usage:')
    print('python manage_vcpkg_archive_cache.py <action> <dir> <os> <bucket> <access_key> <secret_key>')

if (len(sys.argv) != 7 and len(sys.argv) != 5):
    help()
    sys.exit(1)

action_name = sys.argv[1]
dir_name = sys.argv[2]
os_name = sys.argv[3]
bucket_name = sys.argv[4]

if (action_name == 'upload' and len(sys.argv) == 7):
    access_key = sys.argv[5]
    secret_key = sys.argv[6]
    s3.upload(os_name, dir_name, bucket_name, access_key, secret_key)
elif (action_name == 'download'):
    s3.download(os_name, dir_name, bucket_name)
else:
    help()
    sys.exit(1)

sys.exit(0)
