unset https_proxy http_proxy
sudo date --set="$(curl -s --head http://xx.xx.xx.xx:xxx | grep "Date:" |sed 's/Date: [A-Z][a-z][a-z], //g'| sed 's/\r//')"
