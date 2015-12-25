#!/bin/bash
IMAGE=charlie:latest
PROJECT=$(gcloud config list project | sed -n "s/project = //p")

if [ -z "$PROJECT" ]; then
  echo "Unable to determine project."
  exit 1
fi

docker build -t "$IMAGE" .
docker tag -f $IMAGE gcr.io/${PROJECT}/$IMAGE
gcloud docker push gcr.io/${PROJECT}/$IMAGE
