import boto3
import os
from botocore import UNSIGNED
from botocore.client import Config
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
    except Exception:
        print("Upload failed")

def s3_download(local_file, s3_file, bucket):
    print('Downloading', s3_file, '->', local_file)
    s3 = boto3.client('s3', config=Config(signature_version=UNSIGNED))

    try:
        s3.download_file(bucket, s3_file, local_file)
        print("Download Successful")
    except NoCredentialsError:
        print("Credentials not available")
    except Exception:
        print("Download failed")

def s3_list(bucket):
    s3 = boto3.client('s3', config=Config(signature_version=UNSIGNED))

    try:
        return s3.list_objects(Bucket=bucket)['Contents']
    except Exception:
        print("Failed to retrieve list of files in bucket")
        return None

def upload(os_name, dir, bucket, access_key, secret_key):
    objects = s3_list(bucket)
    for path, _, files in os.walk(dir):
        for name in files:
            found = False
            file_path = os.path.join(path, name)
            dir_name = os.path.basename(path)
            file_name = os_name + '-' + dir_name + '-' + name
            if objects:
                for object in objects:
                    key = object['Key']
                    if (key == file_name):
                        found = True
                        break
            if (not found):
                s3_upload(file_path, file_name, bucket, access_key, secret_key)

def cleanup(dir):
    if not os.path.isdir(dir):
        return
    for filename in os.listdir(dir):
        file_path = os.path.join(dir, filename)
        try:
            if os.path.isfile(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                cleanup(file_path)
        except Exception as e:
            print('Failed to delete %s. Reason: %s' % (file_path, e))

def download(os_name, dir, bucket):
    cleanup(dir)
    objects = s3_list(bucket)
    if objects:
        for object in objects:
            key = object['Key']
            args = key.split('-')
            if (len(args) == 3 and args[0] == os_name):
                os.makedirs(os.path.join(dir, args[1]), exist_ok=True)
                local_file = os.path.join(dir, args[1], args[2])
                if (os.path.isfile(local_file)):
                    print('Found local file', local_file)
                    continue
                s3_download(local_file, key, bucket)

def download_all(dir, bucket):
    files = {}
    cleanup(dir)
    os.makedirs(dir, exist_ok=True)
    objects = s3_list(bucket)
    if objects:
        for object in objects:
            key = object['Key']
            local_file = os.path.join(dir, key)
            files[local_file] = object['LastModified']
            s3_download(local_file, key, bucket)
    return files

def s3_remove(s3_file, bucket, access_key, secret_key):
    print('Removing', s3_file)
    s3 = boto3.client('s3', aws_access_key_id=access_key,
                      aws_secret_access_key=secret_key)
    try:
        s3.delete_object(Bucket=bucket, Key=s3_file)
        print("Remove successful")
    except Exception as ex:
        print("Remove failed: ", ex)

def remove_files(files, bucket, access_key, secret_key):
    for file in files:
        s3_remove(os.path.basename(file), bucket, access_key, secret_key)
