#!/bin/bash

if [ -f .env ]; then
  source .env
fi

HOME=${HOME:-$HOME}
WORK=${WORK:-$PWD}

PASSWORD=${PASSWORD:-secret}

echo -e "Workdir on host: $WORK"
echo -e "Building..."
docker build --build-arg PASSWORD=${PASSWORD} -f ./Dockerfile -t work . >/dev/null
echo -e "\rEnjoy your work !"
docker run -ti --rm \
    --name "worker" \
    --hostname "factory" \
    --volume "$WORK":"/home/worker/work" \
    --volume "$HOME/.gitconfig":"/home/worker/.gitconfig" \
    --volume "$HOME/.ssh:/home/worker/.ssh" \
    work