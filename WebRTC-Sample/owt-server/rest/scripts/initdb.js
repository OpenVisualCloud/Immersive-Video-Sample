// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

'use strict';

var cipher = require('./cipher');
var db;

function prepareDB(db_url) {
  db = require('mongojs')(db_url, ['services', 'infos', 'rooms']);
}

function checkVersion() {
  return new Promise((resolve, reject) => {
    db.infos.findOne({_id: 1}, function cb(err, info) {
      if (err) {
        db.close();
        reject('mongodb: error in query info');
      }
      if (info && info.version === '1.0') {
        resolve();
      } else {
        db.close();
        reject('mongodb: invalid info');
      }
    });
  });
}

function prepareService(serviceName) {
  return new Promise((resolve, reject) => {
    db.services.findOne({name: serviceName}, function cb(err, service) {
      if (err || !service) {
        db.close();
        reject('mongodb: error in query service');
      } else {
        if (service.encrypted === true) {
          service.key = cipher.decrypt(cipher.k, service.key);
        }
        resolve(service);
      }
    });
  });
}

async function GetService(db_url) {
  var sample_service;

  prepareDB(db_url);
  await checkVersion()
      .then(() => {return prepareService('sampleService')})
      .then((service) => {
        var sampleServiceId = service._id + '';
        var sampleServiceKey = service.key;
        // console.log('sampleServiceId:', sampleServiceId);
        // console.log('sampleServiceKey:', sampleServiceKey);
        db.close();


        sample_service = {};
        sample_service.sampleServiceId = sampleServiceId;
        sample_service.sampleServiceKey = sampleServiceKey;
      })

  return sample_service;
}

module.exports = {
  GetService: GetService
};
