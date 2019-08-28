#!/bin/sh
cd ..
echo "======prepare to patch...",$1
echo > patch/out
patch -p1 < patch/$1 > patch/out
cat patch/out
echo "======backup patch file..."
mv patch/$1 patch/done/
list=`sed 's|symbolic||' patch/out | grep "patching " | awk '{print $3}'`
#list=`grep "patching file " patch/out | awk '{print $3}'`
#list=`sed -n '/Subject: /,/files changed/p'  patch/done/$1  | sed -n '/---/,/files changed,/p' | grep "|" | awk '{print $1}'`
for i in $list; do
echo "add "$i"..."
git add $i;
done
echo > patch/out
cd patch
grep -E "luminous|luminous:|luminous :|files changed|file changed"  done/$1 -A 2
echo "======commit patch..."
grep  "luminous: "  done/$1 | awk -F "luminous:" '{print "git commit -m \""$2"\""}'
read -p "Press any key to continue." var
cmd=`grep  "luminous: "  done/$1 | awk -F "luminous:" '{print "git commit -m \""$2"\""}'`
echo $cmd
eval $cmd
