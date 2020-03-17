# Build Nginx

```
wget -O - http://nginx.org/download/nginx-1.13.1.tar.gz | tar xz
cd nginx-1.13.1
./configure --with-http_ssl_module
make -j $(nproc)
sudo make install
```
