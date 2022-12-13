# Demo Docker for SSH + PAM Weblogin

# Introduction

This demo can be used to demonstrate the fucntionality of **pam-weblogin** using a representative SSH user login flow.

The idea is to initiate a SSH connection to a container and that the container forces you to complete the pam-welogin as a second factor on top of your provided ssh private key.

## Prerequisites

The following has to be prepared in advance for this demo to function succesfully.

1. You have registered a **service** in SRAM
2. You have set a **service token**
3. The service is enabled for the **PAM weblogin**
4. For this **service** you have enabled LDAP provioning
5. You have set a **LDAP password**
6. You have created at least one **Collaboration** in SRAM that is connected to this **service**
7. There is at least one member in a **Collaboration** connected to this **service** and that user has uploaded at least one **SSH Public Key**

If all above criteria are met, make sure you know the SRAM username of a user that satisfies item [7]

## Prepare **.env**

Copy or rename the sample .env.sample to .env and adjust the values as indicated

The environment variables that can be provided are:

| environment variable  | Default                           | Description                                                                         |
| --------------------- | --------------------------------- | ----------------------------------------------------------------------------------- |
| SERVICE_LDAP_PASSWORD | **none**                          | LDAP Pasword that the SRAM Service Administrator has prepared in SRAM               |
| SERVICE_ENTITY_ID     | **none**                          | Entity ID of the service in SRAM                                                    |
| SERVICE_BEARER_TOKEN  | **none**                          | Service API token that the SRAM Service Administrator has prepared in SRAM (refer # |
| URL                   | https://sram.surf.nl/pam-weblogin | the url where pam-weblogin is hosted                                                |
| LOG_LEVEL             | INFO                              | The log level verbosity level that you prefer (DEBUG, INFO, ERROR, NONE)            |

## Start docker environment

```bash
docker-compose up -d
```

The container is started and if all environment variables provided are valid, then the container will run a synhronisation with SRAM LDAP to provision the members of the CO's that are connected with this service, to valid user accounts within this container.

The first run will happen within a minute, so try this command to see if the user that you exected to be provisioned is already available:

Now ssh into the container via:

```bash
ssh -p 2222 <user>@localhost
```

Where **user** is the SRAM uid of a member of a collaboration having this service linked.

## Alternative: diagnose issues...

If it doesn't work as expected, then you can try to diagnose any issues. For that, step into the running container:

```bash
docker exec -ti demo bash
```

Now you are in the container, from there you are on your own to evaluate the situation. Some hints:

1. Evaluate the contrab

```bash
crontab -e
```

see that the crontab has run at least a single time:

```bash
ls -l /home
```

The should be at least one username available

2. Evaluate the environment variables...

```bash
set
```

4. Evaluate the pam-stack

```bash
cat /etc/pam.d/weblogin
```

```bash
cat /root/pam-weblogin.conf
```

5. Run pamtester...

```bash
pamtester weblogin <user> authenticate
```

where 'user' must be replaced with the username fount in hint [1]
