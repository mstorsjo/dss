#!/bin/sh

if [ $# -lt 1 ]; then
	echo $0 destination
	exit 1
fi

DEST="$1"

mkdir -p $DEST/bin
mkdir -p $DEST/sbin
mkdir -p $DEST/etc/streaming
mkdir -p $DEST/var/streaming/logs
mkdir -p $DEST/var/streaming/playlists
mkdir -p $DEST/var/run
mkdir -p $DEST/movies

cp DarwinStreamingServer $DEST/sbin
cp PlaylistBroadcaster.tproj/PlaylistBroadcaster $DEST/bin
cp MP3Broadcaster/MP3Broadcaster $DEST/bin
cp qtpasswd.tproj/qtpasswd $DEST/bin
cp APIModules/QTSSHomeDirectoryModule/createuserstreamingdir $DEST/bin
cp StreamingLoadTool/StreamingLoadTool $DEST/bin
mkdir -p $DEST/sbin/StreamingServerModules
cp APIModules/QTSSHomeDirectoryModule/QTSSHomeDirectoryModule $DEST/sbin/StreamingServerModules
cp APIModules/QTSSRefMovieModule/QTSSRefMovieModule $DEST/sbin/StreamingServerModules

$DEST/sbin/DarwinStreamingServer -x -c $DEST/etc/streaming/streamingserver.xml

cp qtgroups qtusers $DEST/etc/streaming
cp relayconfig.xml-Sample $DEST/etc/streaming
cp StreamingLoadTool/streamingloadtool.conf $DEST/etc/streaming

cp sample* $DEST/movies

cp Documentation/3rdPartyAcknowledgements.rtf $DEST/var/streaming
cp Documentation/readme.txt $DEST/var/streaming

cp WebAdmin/src/streamingadminserver.pl $DEST/sbin
cp -R WebAdmin/WebAdminHtml $DEST/var/streaming/AdminHtml

username=`whoami`
group=`id -Gn | awk '{print $1}'`

cat<<EOF > $DEST/etc/streaming/streamingadminserver.conf
root = $DEST/var/streaming/AdminHtml
plroot = $DEST/var/streaming/playlists
qtssPort = 7070
qtssName = $DEST/sbin/DarwinStreamingServer -c $DEST/etc/streaming/streamingserver.xml
logfile = $DEST/var/streaming/logs/streamingadminserver.log
qtssQTPasswd = $DEST/bin/qtpasswd
qtssPlaylistBroadcaster = $DEST/bin/PlaylistBroadcaster
qtssMP3Broadcaster = $DEST/bin/MP3Broadcaster
pidfile = $DEST/var/run/streamingadminserver.pid
runUser = $username
runGroup = $group
EOF

cat $DEST/etc/streaming/streamingserver.xml | sed s,/usr/local,$DEST, | sed 's,/etc/streaming,'$DEST'&,' | sed 's,/var,'$DEST'&,' | sed 's,\(run_user_name" >\)\(qtss\),\1'$username, | sed 's,\(run_group_name" >\)\(qtss\),\1'$group, > $DEST/etc/streaming/tmp
mv $DEST/etc/streaming/tmp $DEST/etc/streaming/streamingserver.xml

printf "Enter admin user name: "
read username
$DEST/bin/qtpasswd -f $DEST/etc/streaming/qtusers $username
echo admin: $username > $DEST/etc/streaming/qtgroups

$DEST/bin/qtpasswd -f $DEST/etc/streaming/qtusers -F -d 'aGFja21l' > /dev/null

$DEST/sbin/streamingadminserver.pl -c $DEST/etc/streaming/streamingadminserver.conf

