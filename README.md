
# fixarr

This micro daemon watches on regular bases RADARR/SONARR instances for stalled downloads.
If any found - deletes them from *ARR and torrent client, marks them as failed/blocked and finally starts new search in the *ARR instance.

## Prerequisites

```bash
$ sudo apt install libulfius-dev libjansson-dev
```

## Compilation and Installation

```bash
$ make

$ #install in ~/.local/bin
$ make install

$ #install in /usr/local/bin
$ sudo make install

$ #install systemd service
$ sudo make install_service
```

## Configuration

Search for configuration file is in this sequence:
- ~/fixarr.json
- /etc/fixarr.json
- fixarr.json (current dir)

OR

specified conf file with -c parameter, for example:

```bash
$ fixarr -c /tmp/f.json
```

**hosts** array contains all hosts that can be used

Example **hosts** object:
```javascript
{
	"type": "radarr", //host type, can be radarr or sonarr
	"name": "RADARR", //custom name of the host, default is type of host
	"url": "http://192.168.0.1:7878", //URL of the host
	"apikey": "11111111111111111111111111111111" //APIKEY can be get from http://192.168.0.1:7878/settings/general -> API Key
}
```
**stalled** array contains all hosts that must be checked for stalled downloads

Example **stalled** object:
```javascript
{
	"hostIDs": [ 0, 1 ], //hosts to be used by ID as entered in hosts array
	"enabled": true, //enabled or not, default true
	"minRefreshTime": 5, //maximum minutes betwean every check for new stalled downloads; minimum 1 min
	"zeroStartTimeout": 15, //maximum minutes for a download to get at least 1 byte before considered as stalled; 0 - disabled
	"stalledTimeout": 7200 //maximum minutes for a download to finish before considered as stalled; 0 - disabled
}
```
## Switches

```
 -c, --conf=FILE     Load specified configuration file
 -d, --dry-run       Run in DRY-RUN mode. No real actions are made.
```