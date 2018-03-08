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
rm -rf $DEST/var/streaming/AdminHtml
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
qtssAutoStart = 1
EOF

if [ "`uname`" = "Darwin" ]; then
	cat $DEST/etc/streaming/streamingserver.xml | sed s,/Library/QuickTimeStreaming/Movies,$DEST/movies, | sed s,/Library/QuickTimeStreaming/Modules,$dest/sbin/StreamingServerModules, | sed s,/Library/QuickTimeStreaming/Config,$DEST/etc/streaming, | sed 's,/var,'$DEST'&,' | sed s,/Library/QuickTimeStreaming/Logs,$DEST/var/streaming/logs, | sed 's,\(run_user_name" >\)\(qtss\),\1'$username, | sed 's,\(run_group_name" >\)\(qtss\),\1'$group, > $DEST/etc/streaming/tmp
else
	cat $DEST/etc/streaming/streamingserver.xml | sed s,/usr/local,$DEST, | sed 's,/etc/streaming,'$DEST'&,' | sed 's,/var,'$DEST'&,' | sed 's,\(run_user_name" >\)\(qtss\),\1'$username, | sed 's,\(run_group_name" >\)\(qtss\),\1'$group, > $DEST/etc/streaming/tmp
fi
mv $DEST/etc/streaming/tmp $DEST/etc/streaming/streamingserver.xml

printf "Enter admin user name: "
read username
$DEST/bin/qtpasswd -f $DEST/etc/streaming/qtusers $username
echo admin: $username > $DEST/etc/streaming/qtgroups

$DEST/bin/qtpasswd -f $DEST/etc/streaming/qtusers -F -d 'aGFja21l' > /dev/null

cat<<EOF > $DEST/start.sh
#!/bin/sh
$DEST/sbin/streamingadminserver.pl -c $DEST/etc/streaming/streamingadminserver.conf
EOF
chmod a+x $DEST/start.sh

cat<<EOF > $DEST/stop.sh
#!/bin/sh
user=\$(whoami)
for i in \$(ps auxw | grep \$user | grep streamingadminserver.pl | grep -v grep | awk '{print \$2}'); do
	kill \$i
done
for i in \$(ps auxw | grep \$user | grep DarwinStreamingServer | grep -v grep | awk '{print \$2}' | head -1); do
	kill \$i
done
EOF
chmod a+x $DEST/stop.sh

$DEST/start.sh

