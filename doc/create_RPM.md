# Creating an RPM for the pam-weblogin plugin

### 1. Setup rpmbuild

```
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
if [ -f ~/.rpmmacros ];then
    echo '~/.rpmmacros already exists.'
else
    echo '%_topdir %(echo $HOME)/rpmbuild' > ~/.rpmmacros
    echo 'Initialized ~/.rpmmacros'
fi
```

### 2. Download and unpack the source code

```
wget https://github.com/SURFscz/pam-weblogin/archive/refs/tags/${version}.tar.gz
tar -xvzf pam-weblogin-${version}.tar.gz
```

NOTE: this assumes https://github.com/SURFscz/pam-weblogin/issues/16 is fixed.
If not the `json-parser` subdir will be empty and you will need to fetch the source code for the JSON parser manually.

### 3. Check/update version/release number

Update the  ```pam-weblogin.spec``` file.

 * Check if the ```Version``` number is correct.
   Update if necessary.
   Note that for ```rpmbuild``` the version number cannot contain dashes,
   so something like `v0.1-beta` won't work.
 * Check if
   ```
   %define rel 1
   ```
   is correct. The first time a specific version is packaged as RPM, this should be `1`.
   If the same version number (hence unchanged source code) is packaged again, then `rel` must be incremented.
 * Make sure the version number specified in the `*.spec` file matches the version number in the folder with unpacked source code.
   Rename the folder with unpacked source code if necessary.


### 4. Create new tar.gz source code archive with patched *.spec file

```
tar -cvjf ~/rpmbuild/SOURCES/pam-weblogin-${version}.tar.gz  pam-weblogin-${version}.tar.gz
```

### 5. Build RPMs

```
rpmbuild -ta ~/rpmbuild/SOURCES/pam-weblogin-${version}.tar.gz
```
When successful, the RPMs can be found in ```~/rpmbuild/RPMS/```
