# Docker for sandbox development

You can develop the module in a sandbox envrionment. This sandbox environment contains all dependencies needed for compilation and testing the module.

## Prepare **.env**

Create local **.env** file following keys:

```bash
URL=< your websso url >
TOKEN=< bearer token for websso service >
ATTRIBUTE=uid
RETRIES=3
```

## Prepare **.env**

Copy or rename the sample .env.sample to .env and adjust the values as indicated

The environment variables that can be provided are:

| environment variable | Default                           | Description                                                                                                                                                   |
| -------------------- | --------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| service in SRAM      |
| TOKEN                | **none**                          | Service API token that the SRAM Service Administrator has prepared in SRAM #                                                                                  |
| URL                  | https://sram.surf.nl/pam-weblogin | the url where pam-weblogin is hosted                                                                                                                          |
|                      |
| ATTRIBUTE            | email                             | The Attribute in the user introspection that you want as the username                                                                                         |
| CACHE_DURATION       | 30                                | How many minutes do we want a succesfull weblogin to be cached at the server side. Follow up pam-weblogin requests within this timefram will respond success. |
| RETRIES              | 3                                 | how many times the user may input an invalid pincode                                                                                                          |
| USERNAME             | unspecified                       | If you want to prefill the username to test with, you may set this value                                                                                      |
|                      |

## Start docker environment

```bash
docker-compose up
```

Now start bash session in container:

```bash
docker exec -ti sandbox bash
```

From here you can compile and build the module, for example:

```bash
make
```

You may test the module via:

```bash
make
sudo -E make install
pamtester weblogin <user> authenticate
```

Where <user> is the username you want to use the SRAM username for introspection.
