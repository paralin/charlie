#!/bin/bash

BASE64_ENC=$(cat hidden_service/hostname | base64 --wrap=0)
sed -e "s#{{hostname_data}}#${BASE64_ENC}#g" ./torhost-hsconfig-template.yaml > torhost-hsconfig.yaml

BASE64_ENC=$(cat hidden_service/private_key | base64 --wrap=0)
sed -i -e "s#{{private_key_data}}#${BASE64_ENC}#g" ./torhost-hsconfig.yaml
