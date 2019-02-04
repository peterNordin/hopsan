#!/usr/bin/env python

import argparse
import hashlib
import shutil
import sys
import os
import platform
import tarfile
import zipfile
import xml.etree.ElementTree as ET

if sys.version_info.major == 2:
    import urllib
else:
    import urllib.request

def parse_deps_choice(args):
    download_all = False
    dependencies = list()
    for arg in args:
        if arg == 'all':
            download_all = True
        else:
            dependencies.append(arg)
    return (download_all, dependencies)

def split_name_version(full_name):
    parts = full_name.split(':')
    name = str()
    version = str()
    if len(parts) >= 1:
        name = parts[0]
    if len(parts) >= 2:
        version = parts[1]
    return (name, version)

def get_platform(releasefile_element):
    if 'platform' in releasefile_element.attrib:
        return releasefile_element.attrib['platform']
    return str()

def get_version(dep_element):
    if 'version' in dep_element.attrib:
        return dep_element.attrib['version']
    return str()

def get_dep_names(deps_element):
    names = list()
    for dep in deps_element:
        dep_name = dep.attrib['name']
        dep_version = get_version(dep)
        if dep_version:
            dep_name = dep_name + ':' + dep_version
        names.append(dep_name)
    return names

def check_choices(choices, dep_names):
    for choice in choices:
        found_names = list()
        found = False
        for full_name in dep_names:
            dname, dversion = split_name_version(full_name)
            cname, cversion = split_name_version(choice)
            if dname == cname:
                found_names.append(full_name)
                if dversion == cversion:
                    found = True
                    break
        if not found:
            if len(found_names) > 0:
                print('Warning: '+choice+' does not match! '+'Alternatives are: '+' '.join(found_names))
            else:
                print('Warning: '+choice+' does not match! There are no alternatives.')


def match_choice(name, version, choices, allow_missing):
    found_name = False
    found_version = False
    for choice in choices:
        cname, cversion = split_name_version(choice)
        if cname == name:
            found_name = True
        if cversion and cversion == version:
            found_version = True
    if found_name and found_version:
        return True
    elif not version and found_name:
        return True
    elif allow_missing and not found_name:
        return True
    return False

def match_platform(platform_name):
    actual = platform.system().lower()
    return platform_name == actual

def file_name(path):
    return os.path.basename(path)

def read_expected_hash(releasefile_element):
    algos = ('md5', 'sha1', 'sha256')
    for algo in algos:
        if algo in releasefile_element.attrib:
            return (algo, releasefile_element.attrib[algo])

def hashsum_file(filepath, algorithm):
    with  open(filepath, 'rb') as fileobj:
        hashobj = hashlib.new(algorithm)
        hashobj.update(fileobj.read())
        return hashobj.hexdigest()

def verify_filehash(filepath, algorithm, hashsum):
    return hashsum_file(filepath, algorithm) == hashsum

def download(url, dst, hash_algo, expected_hashsum):
    print('Info: Downloading '+url)
    try:
        if sys.version_info.major == 2:
            tempfile, headers = urllib.urlretrieve(url)
        else:
            tempfile, headers = urllib.request.urlretrieve(url)
        if verify_filehash(tempfile, hash_algo, expected_hashsum):
            shutil.copy2(tempfile, dst)
            print('Info: Success downloading and verifying '+dst)
            return True
        else:
            print(headers)
            print('Error: The download failed, invalid checksum of resulting file')
            # todo delete tempfile
    except Exception as e:
        print('Error: Could not download file due to connection error')
        print(str(e))

    return False

def check_download_releasefile(dep_name, releasefile):
    hash_algo, expected_hashsum = read_expected_hash(releasefile)
    for url_element in releasefile:
        url = url_element.text
        fname = file_name(url)
        if dep_name not in fname.lower():
            fname=dep_name+'-'+fname
        if os.path.isfile(fname) and not args.force:
            print('Info: file already exists '+fname)
            if verify_filehash(fname, hash_algo, expected_hashsum):
                return (fname, False)
            else:
                print('Warning: ' + hash_algo + ' missmatch in file ' + fname)
                print('Expected: ' + expected_hashsum)
                print('Actual: ' + hashsum_file(fname, hash_algo))
                #                            download(url, fname)
        else:
            isok = download(url, fname, hash_algo, expected_hashsum)
            if isok:
                return (fname, True)
    return ("", False)

def unpack(archive_file, dep_name):
    codedir = dep_name+'-code'
    builddir = dep_name+'-build'
    if os.path.exists(codedir):
        print('Info: Removing existing directory ' + codedir)
        shutil.rmtree(codedir)
    if os.path.exists(builddir):
        print('Info: Removing existing directory ' + builddir)
        shutil.rmtree(builddir)
    #    shutil.unpack_archive(archive_file, codedir)
    print('Info: Unpacking '+archive_file)
    if archive_file.endswith('.zip'):
        with zipfile.ZipFile(archive_file) as zipf:
            zipf.extractall(codedir)
    else:
        with tarfile.open(archive_file, 'r:*') as tarf:
            tarf.extractall(codedir)

    # Figure out the name of the unpacked root dir
    root_dir_name = str()
    for path, subdirs, files  in  os.walk(codedir):
        if len(subdirs) == 1 and len(files) == 0:
            root_dir_name = subdirs[0]
        break

    # Move all content from the root into codedir
    root_path = os.path.join(codedir, root_dir_name)
    for path, subdirs, files  in  os.walk(root_path):
        for f in files:
            shutil.move(os.path.join(root_path, f), codedir)
        for d in subdirs:
            shutil.move(os.path.join(root_path, d), codedir)
        break
    shutil.rmtree(root_path)

if __name__ == "__main__":

    argparser = argparse.ArgumentParser()
    argparser.add_argument('--force', dest='force', action='store_true',
                           help='Force download even if file exists, code and build directory will be reset')
    argparser.add_argument('dependency_name', nargs='+', action='append', type=str, help='Dependencies to download, or "all" for all of them')
    argparser.set_defaults(force=False)

    args = argparser.parse_args()

    download_all, chosen_deps  = parse_deps_choice(args.dependency_name[0])

    tree = ET.parse('dependencies.xml')
    root = tree.getroot()
    all_dep_names = get_dep_names(root)
    check_choices(chosen_deps, all_dep_names)

    dependencies = list()
    allready_added_names = list()
    for dep in root:
        dep_name = dep.attrib['name']
        dep_version = get_version(dep)
        found_match = match_choice(dep_name, dep_version, chosen_deps, download_all)
        if found_match and not dep_name in allready_added_names:
            dependencies.append(dep)
            allready_added_names.append(dep_name)

    for dep in dependencies:
        dep_name = dep.attrib['name']
        print('----------'+dep_name+'------------------------------')
        files_matching_platform = list()
        files_without_platform = list()
        for relfile in dep:
            if relfile.tag == 'releasefile':
                platform_restriction = get_platform(relfile)
                if match_platform(platform_restriction):
                    files_matching_platform.append(relfile)
                elif platform_restriction == "":
                    files_without_platform.append(relfile)
        # If no file specified explicitly for this platform, use file with no platform specifier
        if (len(files_matching_platform) == 0):
            files_matching_platform = files_without_platform

        for relfile in files_matching_platform:
            new_file, did_download = check_download_releasefile(dep_name, relfile)
            if did_download or new_file and not os.path.exists(dep_name+'-code'):
                unpack(new_file, dep_name)
