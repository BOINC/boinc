import boto3
import os
import sys
from botocore.exceptions import NoCredentialsError

def s3_upload(local_file, s3_file, bucket, access_key, secret_key):
    print('Uploading', local_file, '->', s3_file)

    s3 = boto3.client('s3', aws_access_key_id=access_key,
                      aws_secret_access_key=secret_key)

    try:
        s3.upload_file(local_file, bucket, s3_file)
        print("Upload Successful")
    except FileNotFoundError:
        print("The file was not found")
    except NoCredentialsError:
        print("Credentials not available")

def s3_download(local_file, s3_file, bucket, access_key, secret_key):
    print('Downloading', s3_file, '->', local_file)
    s3 = boto3.client('s3', aws_access_key_id=access_key,
                      aws_secret_access_key=secret_key)

    try:
        s3.download_file(bucket, s3_file, local_file)
        print("Download Successful")
        return True
    except NoCredentialsError:
        print("Credentials not available")
        return False

def s3_list(bucket, access_key, secret_key):
    s3 = boto3.client('s3', aws_access_key_id=access_key,
                      aws_secret_access_key=secret_key)

    try:
        return s3.list_objects(Bucket=bucket)['Contents']
    except Exception:
        print("Failed to retrieve list of files in bucket")
        return None

def upload(os_name, dir, bucket, access_key, secret_key):
    for path, _, files in os.walk(dir):
        for name in files:
            file_path = os.path.join(path, name)
            dir_name = os.path.basename(path)
            file_name = os_name + '-' + dir_name + '-' + name
            l = s3_list(bucket, access_key, secret_key)
            if (l is not None):
                for k in l:
                    key = k['Key']
                    if (key == file_name):
                        return                
            s3_upload(file_path, file_name, bucket, access_key, secret_key)

def download(os_name, dir, bucket, access_key, secret_key):
    l = s3_list(bucket, access_key, secret_key)
    if (l is not None):
        for k in l:
            key = k['Key']
            a = key.split('-')
            if (len(a) == 3 and a[0] == os_name):
                os.makedirs(os.path.join(dir, a[1]), exist_ok=True)
                local_file = os.path.join(dir, a[1], a[2])
                if (os.path.isfile(local_file)):
                    return
                s3_download(local_file, key, bucket, access_key, secret_key)

def help():
    print('Usage:')
    print('python manage_vcpkg_archive_cache.py <action> <dir> <os> <bucket> <access_key> <secret_key>')

if (len(sys.argv) != 7):
    help()
    sys.exit(1)

action_name = sys.argv[1]
dir_name = sys.argv[2]
os_name = sys.argv[3]
bucket_name = sys.argv[4]
access_key = sys.argv[5]
secret_key = sys.argv[6]

if (action_name == 'upload'):
    upload(os_name, dir_name, bucket_name, access_key, secret_key)
elif (action_name == 'download'):
    download(os_name, dir_name, bucket_name, access_key, secret_key)
else:
    help()
    sys.exit(1)

sys.exit(0)
