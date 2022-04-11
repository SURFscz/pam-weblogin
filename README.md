# pam-websso-c
PAM WebSSO module in C

## Installation

On Debian/Ubuntu:
~~~
sudo apt-get install build-essential
sudo apt-get install autoconf
sudo apt-get install shtool
sudo apt-get install libcurl4-gnutls-dev
sudo apt-get install libgcrypt20 libgcrypt20-dev
sudo apt-get install libldap2-dev
~~~

On Fedora/Redhat/CentOS you will need:
~~~
sudo yum groupinstall 'Development Tools'
sudo yum install shtool
sudo yum install pam-devel
sudo yum install libcurl curl-devel
sudo yum install libgcrypt libgcrypt-devel
~~~

## shtool

Configure needs shtool in order to be able to install shared libraries

~~~
ln -s /usr/bin/shtool .
~~~
## autoconf

Make sure youy have recent files **config.guess** and **config.sub**.
This is required for configure to correctly match your system.

Refer: https://www.gnu.org/software/gettext/manual/html_node/config_002eguess.html

~~~
$ wget -O config.guess 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD'
$ wget -O config.sub 'https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD'
~~~

Now run autoconf & configure:

~~~
autconf 
./configure
~~~

And now you can build the module:

~~~
make
~~~

## Install

The module needs to be installed in the system, directory of pam. **/etc/pam.d**

Add this file to ```/etc/pam.d/websso```

~~~
auth required pam_websso.so /etc/pam-websso.conf
~~~

Note:
Command **make install** does exactly that
## Testing

Add pam-websso.conf configuration file to ```/etc/pam-websso.conf```

~~~
url = http://localhost:5001
~~~

Create python virtualenv, pip install Flask and run websso-daemon.py as a stub server on localhost:5001

Install pamtester to test the module

~~~
$ pamtester websso martin authenticate
Authenticate
config: /etc/pam-websso.conf
config 'url' -> 'http://localhost:5001'
cfg->url: http://localhost:5001
http URL: http://localhost:5001/req
http data: {"user":"martin"}
req: {"nonce": "ji1bI8RJ", "pin": "6333", "challenge": "Hello martin. To continue, visit http://localhost:5001/login/ji1bI8RJ and enter pin:", "hot": false}
Hello martin. To continue, visit http://localhost:5001/login/ji1bI8RJ and enter pin:
Pin: 6333
Pin matched!
http URL: http://localhost:5001/auth
http data: {"nonce":"ji1bI8RJ"}
auth: {"uid": "martin", "result": "SUCCESS"}
user: martin
auth_result: SUCCESS
pamtester: successfully authenticated
~~~
