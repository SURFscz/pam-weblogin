#!/usr/bin/env python
from __future__ import print_function

import os
import ldap
import json
import logging

from datetime import datetime

# Setup logging
log_level = os.environ.get('LOG_LEVEL', 'INFO')
logging.basicConfig(
    level=logging.getLevelName(log_level),
    format='%(asctime)s %(levelname)s %(message)s')
logger = logging.getLogger('root')

class Ldap(object):

    def __init__(self):
        # Establish connection with LDAP...
        try:
            self.page_size = int(os.environ.get('LDAP_PAGE_SIZE', 50))

            self.session = ldap.initialize(os.environ['LDAP_HOST'])
            self.session.simple_bind_s(
                os.environ['LDAP_BIND_DN'],
                os.environ['LDAP_PASSWORD']
            )

        except Exception as e:
            logger.error("Problem connecting to LDAP {} error: {}".format(os.environ['LDAP_HOST'], str(e)))
            exit(1)

        self.people = {}
        self.groups = {}

    def __enter__(self):
        self.get_people()
        self.get_groups()

        return self

    def __exit__(self, exception_type, exception_value, traceback):
        self.session.unbind_s()

    def __repr__(self):
        return json.dumps(self.json(), indent=4, sort_keys=True)

    def json(self):
        return {
            'people': self.people,
            'groups': self.groups
        }

    def search(self, dn, searchScope=ldap.SCOPE_SUBTREE,
            searchFilter="(objectclass=*)",
            retrieveAttributes=[]):

        try:
            page_control = ldap.controls.SimplePagedResultsControl(True, size=self.page_size, cookie='')
            result = []
    
            while True:
    
                page_id = self.session.search_ext(
                    dn, searchScope,
                    searchFilter,
                    retrieveAttributes,
                    serverctrls=[page_control]
                )
                _, result_data, _, serverctrls = self.session.result3(page_id)

                
                for r in result_data:
                    result.append([r])

                controls = [
                    control for control in serverctrls
                    if control.controlType == ldap.controls.SimplePagedResultsControl.controlType
                ]

                if not controls:
                    logger.error('The server ignores RFC 2696 control')
                    
                if not controls[0].cookie:
                    break

                page_control.cookie = controls[0].cookie

        except ldap.LDAPError as e:
            result = None
            logger.error("[LDAP] SEARCH: '%s' ERROR: %s\n" % (dn, str(e)))
            exit(-1)

        return result

    @staticmethod
    def get_attributes(x):
        attributes = {}

        for a in x.keys():
            attributes[a] = []
            for v in x[a]:
                attributes[a].append(v.decode())

        return attributes

    def get_people(self):
        ldap_user_key = os.environ.get('LDAP_USER_KEY', 'uid')

        for i in self.search(
                os.environ.get('LDAP_BASE_DN',''),
                searchFilter="(&(objectClass=inetOrgPerson)({}=*))".format(ldap_user_key),
                retrieveAttributes=[]):

            attributes = self.get_attributes(i[0][1])

            if ldap_user_key not in attributes:
                logger.error("Missing '{}' attribute in LDAP USER Object !".format(ldap_user_key))
                continue

            if len(attributes[ldap_user_key]) > 1:
                logger.error("LDAP User key '{}' must be 1 value !".format(ldap_user_key))
                continue

            key = attributes[ldap_user_key][0]

            self.people[key] = {
                'attributes': attributes
            }

    def get_groups(self):
        ldap_group_key = os.environ.get('LDAP_GROUP_KEY', 'cn')

        for i in self.search(
            os.environ.get('LDAP_BASE_DN',''),
            searchFilter="({})".format(
                    os.environ.get(
                        'LDAP_FILTER', "objectClass=groupOfMembers"
                    )
                ),
            retrieveAttributes=[]):

            attributes = self.get_attributes(i[0][1])

            if ldap_group_key not in attributes:
                logger.error("Missing '{}' attribute in LDAP GROUP Object !".format(ldap_group_key))
                continue

            if len(attributes[ldap_group_key]) > 1:
                logger.error("LDAP Group key '{}' must be 1 value !".format(ldap_group_key))
                continue

            key = attributes[ldap_group_key][0]

            members = []

            if 'member' in attributes:

                for member in attributes['member']:

                    m = member.split(',')[0].split('=')[1]

                    if m not in self.people:
                        logger.error("Member {} not in LDAP People !".format(m))
                        continue

                    members.append(m)

            attributes['member'] = members

            self.groups[key] = {
                'attributes': attributes
            }


def create_user(name, attributes):

    def execute(command):
        logger.debug(f"EXEC: {command}")
        os.system(command)
        
    execute(f"useradd -m {name} --shell /bin/bash 2>/dev/null")

    execute(f"su - {name} -c \"mkdir -p .ssh\"")
    execute(f"su - {name} -c \"cat /dev/null > .ssh/authorized_keys\"")
    execute(f"su - {name} -c \"chmod 600 .ssh/authorized_keys\"")

    for pk in attributes.get('attributes', {}).get('sshPublicKey', []):
        execute(f"su - {name} -c \"echo '{pk}' >> .ssh/authorized_keys\"")

def sync():
    start_time = datetime.now()
    logger.info("SYNC started at: {}".format(start_time))

    with Ldap() as my_ldap:

        for username, attributes in my_ldap.people.items():
            create_user(username, attributes)

    logger.info("SYNC completed at: {}".format(start_time))


if __name__ == "__main__":
    sync()
