module.exports = {
    'serverUrl': 'https://localhost:3000',
    'url':'',
    'streaminId': '',
    'updateRoomOptions': {
        "mediaOut": {
            "video": {
                "parameters": {
                    "keyFrameInterval": [

                        30,
                        5,
                        2,
                        1
                    ],
                    "bitrate": [
                        "x0.8",
                        "x0.6",
                        "x0.4",
                        "x0.2"
                    ],
                    "framerate": [
                        6,
                        12,
                        15,
                        24,
                        30,
                        48,
                        60
                    ],
                    "resolution": [
                        "x3/4",
                        "x2/3",
                        "x1/2",
                        "x1/3",
                        "x1/4",
                        "hd1080p",
                        "hd720p",
                        "svga",
                        "vga",
                        "qvga",
                        "cif"
                    ]
                },
                "format": [
                    {
                        "codec": "vp8"
                    },
                    {
                        "profile": "CB",
                        "codec": "h264"
                    },
                    {
                        "codec": "vp9"
                    },
                    {
                        "codec": "h265"
                    }
                ]
            },
            "audio": [
                {
                    "channelNum": 2,
                    "sampleRate": 48000,
                    "codec": "opus"
                },
                {
                    "sampleRate": 16000,
                    "codec": "isac"
                },
                {
                    "sampleRate": 32000,
                    "codec": "isac"
                },
                {
                    "channelNum": 1,
                    "sampleRate": 16000,
                    "codec": "g722"
                },

                {
                    "codec": "pcmu"
                },
                {
                    "channelNum": 2,
                    "sampleRate": 48000,
                    "codec": "aac"
                },
                {
                    "codec": "ac3"
                },
                {
                    "codec": "nellymoser"
                },
                {
                    "codec": "ilbc"
                }
            ]
        },
        "mediaIn": {
            "video": [
                {
                    "codec": "h264"
                },
                {
                    "codec": "vp8"
                },
                {
                    "codec": "vp9"
                },
                {
                    "codec": "h265"
                }
            ],
            "audio": [
                {
                    "channelNum": 2,
                    "sampleRate": 48000,
                    "codec": "opus"
                },
                {
                    "sampleRate": 16000,
                    "codec": "isac"
                },
                {
                    "sampleRate": 32000,
                    "codec": "isac"
                },
                {
                    "channelNum": 1,
                    "sampleRate": 16000,
                    "codec": "g722"
                },
                {
                    "codec": "pcma"
                },
                {
                    "codec": "pcmu"
                },
                {
                    "codec": "aac"
                },
                {
                    "codec": "ac3"
                },
                {
                    "codec": "nellymoser"
                },
                {
                    "codec": "ilbc"
                }
            ]
        },
        "views": [
            {
                "video": {
                    "layout": {
                        "templates": {
                            "custom": [],
                            "base": "fluid"
                        },
                        "fitPolicy": "letterbox"
                    },
                    "keepActiveInputPrimary": false,
                    "bgColor": {
                        "b": 0,
                        "g": 0,
                        "r": 0
                    },
                    "motionFactor": 0.8,
                    "maxInput": 16,
                    "parameters": {
                        "keyFrameInterval": 100,
                        "framerate": 30,
                        "resolution": {
                            width:3840,height:2048
                        }
                    },
                    "format": {
                        "codec": "vp9"
                    }
                },
                "audio": {
                    "vad": true,
                    "format": {
                        "codec": "opus",
                        "sampleRate": 48000,
                        "channelNum": 2
                    }
                },
                "label": "common"
            }
        ],
    }
}
