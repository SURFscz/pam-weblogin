# Docker for sandbox development

You can develop the module in a sandbox envrionment. This sandbox environment contains all dependencies needed for compilation and testing the module.

## Prepare **.env**

Create local **.env** file following keys:

```
URL=< your websso url >
TOKEN=< bearer token for websso service >
ATTRIBUTE=uid
RETRIES=3
```

## Start docker environment

```
docker-compose up
```

Now start bash session in container:

```
docker exec -ti sandbox bash
```

