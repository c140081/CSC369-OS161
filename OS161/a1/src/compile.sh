mkdir -p $HOME/cscc69 && mkdir -p $HOME/cscc69/root &&
PS=$HOME/a1/src
PR=$HOME/cscc69/root
rm -rf $HOME/cscc69/root &&
./configure --ostree=$PR&&
cd kern/conf &&
./config ASST1 &&
cd ../compile/ASST1 &&
bmake depend 1>/dev/null &&
bmake 1>/dev/null  &&
bmake install 1>/dev/null &&
cd ../../.. &&
bmake 1>/dev/null &&
bmake install 1>/dev/null
cp $PS/sys161.conf $PR/
cp $PS/.gdbinit $PR/ &&
echo done
