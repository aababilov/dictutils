#!/bin/bash

cp $1 wf.txt
WFILE=wf.txt

sed -i 's/$/ /g' $WFILE
sed -i 's/^\([^~\t]\)/\t\1/g' $WFILE
sed -i 's/^\t/\t /g' $WFILE
sed -i 's/p\.p\./ /g' $WFILE
sed -i 's/\./ /g' $WFILE
sed -i 's/(/ /g' $WFILE
sed -i 's/)/ /g' $WFILE
sed -i 's/\[/ /g' $WFILE
sed -i 's/\]/ /g' $WFILE
sed -i 's/,/ /g' $WFILE
sed -i 's/!/ /g' $WFILE
sed -i 's/-/ /g' $WFILE
sed -i 's/;/ /g' $WFILE
sed -i 's/’/ /g' $WFILE

sed -i 's/+ avere/ /g' $WFILE
sed -i 's/+ essere/ /g' $WFILE
sed -i 's/[0-9]/ /g' $WFILE
sed -i 's/[а-яА-ЯёЁ]/ /g' $WFILE
sed -i 's/\(~[^ ]*\).*/\1/' $WFILE

for i in pres press res la past ind rem fut imperf cong condiz imperat pass ger part vt vi v io tu lui noi voi loro lei mi ti si Si ci e I II III a qc qd vr fig firma un ed
do
    echo "\`$i'"
    sed -i 's/ '$i' / /g'  $WFILE
done

sed -i 's/ [ \t]*/ /g' $WFILE