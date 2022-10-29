import os
import hashlib
import platform
import shutil
import ssl
import subprocess
import sys
import tarfile
import re
import urllib
import urllib.request
import zipfile
import tempfile
import time
import functools

print = functools.partial(print, flush=True)

def scriptRelative(*paths):
    scriptdir = os.path.dirname(os.path.realpath(sys.argv[0]))
    result = os.path.join(scriptdir, *paths)
    result = os.path.realpath(result)
    result = os.path.normcase(result)
    return result


def recursiveFileList(startPath, excludeNamePattern=None ):
    result = []
    if os.path.isfile(startPath):
        result.append(startPath)
    elif os.path.isdir(startPath):
        for dirName, subdirList, fileList in os.walk(startPath):
            for fname in fileList:
                if excludeNamePattern and re.match(excludeNamePattern, fname):
                    continue
                result.append(os.path.realpath(os.path.join(startPath, dirName, fname)))
    result.sort()
    return result


def executeSubprocessCapture(processArgs):
    processResult = subprocess.run(processArgs, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if (0 != processResult.returncode):
        raise RuntimeError('Call to "{}" failed.\n\narguments:\n{}\n\nstdout:\n{}\n\nstderr:\n{}'.format(
            processArgs[0],
            ' '.join(processArgs[1:]), 
            processResult.stdout.decode('utf-8'),
            processResult.stderr.decode('utf-8')))
    return processResult.stdout.decode('utf-8')

def executeSubprocess(processArgs, folder=None, env=None):
    restoreDir = None
    if folder != None:
        restoreDir = os.getcwd()
        os.chdir(folder)

    process = subprocess.Popen(
        processArgs, stdout=sys.stdout, stderr=sys.stderr, env=env)
    process.wait()

    if (0 != process.returncode):
        raise RuntimeError('Call to "{}" failed.\n\narguments:\n{}\n'.format(
            processArgs[0],
            ' '.join(processArgs[1:]),
            ))

    if restoreDir != None:
        os.chdir(restoreDir)


def hashFile(file, hasher = hashlib.sha512()):
    with open(file, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hasher.update(chunk)
    return hasher.hexdigest()

# Assumes input files are in deterministic order
def hashFiles(filenames):
    hasher = hashlib.sha256()
    for filename in filenames:
        with open(filename, "rb") as f:
            for chunk in iter(lambda: f.read(4096), b""):
                hasher.update(chunk)
    return hasher.hexdigest()

def hashFolder(folder):
    filenames = recursiveFileList(folder)
    return hashFiles(filenames)

def downloadFile(url, hash=None, hasher=hashlib.sha512(), retries=3):
    for i in range(retries):
        tempFileName = None
        # OSX Python doesn't support SSL, so we need to bypass it.  
        # However, we still validate the downloaded file's sha512 hash
        if 'Darwin' == platform.system():
            tempFileDescriptor, tempFileName = tempfile.mkstemp()
            context = ssl._create_unverified_context()
            with urllib.request.urlopen(url, context=context) as response, open(tempFileDescriptor, 'wb') as tempFile:
                shutil.copyfileobj(response, tempFile)
        else:
            tempFileName, headers = urllib.request.urlretrieve(url)

        downloadHash = hashFile(tempFileName, hasher)
        # Verify the hash
        if hash is not None and hash != downloadHash:
            print("Try {}: Downloaded file {} hash {} does not match expected hash {} for url {}".format(i + 1, tempFileName, downloadHash, hash, url))
            os.remove(tempFileName)
            continue
        return tempFileName

    raise RuntimeError("Downloaded file hash {} does not match expected hash {} for\n{}".format(downloadHash, hash, url))


def downloadAndExtract(url, destPath, hash=None, hasher=hashlib.sha512(), isZip=False):
    tempFileName = downloadFile(url, hash, hasher)
    if isZip or ".zip" in url:
        with zipfile.ZipFile(tempFileName) as zip:
            zip.extractall(destPath)
    else:
        # Extract the archive
        with tarfile.open(tempFileName, 'r:gz') as tgz:
            def is_within_directory(directory, target):
                
                abs_directory = os.path.abspath(directory)
                abs_target = os.path.abspath(target)
            
                prefix = os.path.commonprefix([abs_directory, abs_target])
                
                return prefix == abs_directory
            
            def safe_extract(tar, path=".", members=None, *, numeric_owner=False):
            
                for member in tar.getmembers():
                    member_path = os.path.join(path, member.name)
                    if not is_within_directory(path, member_path):
                        raise Exception("Attempted Path Traversal in Tar File")
            
                tar.extractall(path, members, numeric_owner=numeric_owner) 
                
            
            safe_extract(tgz, destPath)
    os.remove(tempFileName)
