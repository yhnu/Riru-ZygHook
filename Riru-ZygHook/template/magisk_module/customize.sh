SKIPUNZIP=1

# extract verify.sh
ui_print "- Extracting verify.sh"
unzip -o "$ZIPFILE" 'verify.sh' -d "$TMPDIR" >&2
if [ ! -f "$TMPDIR/verify.sh" ]; then
  ui_print    "*********************************************************"
  ui_print    "! Unable to extract verify.sh!"
  ui_print    "! This zip may be corrupted, please try downloading again"
  abort_clean "*********************************************************"
fi
. $TMPDIR/verify.sh

# extract riru.sh
extract "$ZIPFILE" 'riru.sh' "$MODPATH"
. $MODPATH/riru.sh

check_riru_version
check_architecture

# extract libs
ui_print "- Extracting module files"

extract "$ZIPFILE" 'module.prop' "$MODPATH"
extract "$ZIPFILE" 'post-fs-data.sh' "$MODPATH"
extract "$ZIPFILE" 'uninstall.sh' "$MODPATH"
#extract "$ZIPFILE" 'sepolicy.rule' "$MODPATH"

if [ "$ARCH" = "x86" ] || [ "$ARCH" = "x64" ]; then
  ui_print "- Extracting x86 libraries"
  extract "$ZIPFILE" "system_x86/lib/libriru_$RIRU_MODULE_ID.so" "$MODPATH"
  mv "$MODPATH/system_x86/lib" "$MODPATH/system/lib"

  if [ "$IS64BIT" = true ]; then
    ui_print "- Extracting x64 libraries"
    extract "$ZIPFILE" "system_x86/lib64/libriru_$RIRU_MODULE_ID.so" "$MODPATH"
    mv "$MODPATH/system_x86/lib64" "$MODPATH/system/lib64"
  fi
else
  ui_print "- Extracting arm libraries"
  extract "$ZIPFILE" "system/lib/libriru_$RIRU_MODULE_ID.so" "$MODPATH"

  if [ "$IS64BIT" = true ]; then
    ui_print "- Extracting arm64 libraries"
    extract "$ZIPFILE" "system/lib64/libriru_$RIRU_MODULE_ID.so" "$MODPATH"
  fi
fi

# Riru files
ui_print "- Extracting extra files"
[ -d "$RIRU_MODULE_PATH" ] || mkdir -p "$RIRU_MODULE_PATH" || abort_clean "! Can't create $RIRU_MODULE_PATH"

# set permission just in case
set_perm "$RIRU_PATH" 0 0 0700
set_perm "$RIRU_PATH/modules" 0 0 0700
set_perm "$RIRU_MODULE_PATH" 0 0 0700
set_perm "$RIRU_MODULE_PATH/bin" 0 0 0700

rm -f "$RIRU_MODULE_PATH/module.prop.new"
extract "$ZIPFILE" 'riru/module.prop.new' "$RIRU_MODULE_PATH" true
set_perm "$RIRU_MODULE_PATH/module.prop.new" 0 0 0600

# set permissions
ui_print "- Setting permissions"
set_perm_recursive "$MODPATH" 0 0 0755 0644

ui_print "RIRU_PATH = $RIRU_PATH"
ui_print "RIRU_MODULE_PATH = $RIRU_MODULE_PATH"
ui_print "MODPATH = $MODPATH"


# DATA_PATH="/sdcard/sohook"
# SO_NAME="com.lingdong.tv.so"
# mkdir -p "$DATA_PATH"
# cp "/sdcard/$SO_NAME" "$DATA_PATH/$SO_NAME"

# set_perm_recursive $DATA_PATH  1000 1000 0755 0777 u:object_r:system_data_file:s0
# ui_print "DATA_PATH = $DATA_PATH"
# ui_print "TMPDIR = $TMPDIR"

ui_print "- Setting permissions /data/local/tmp 0 0 0755 0644"
set_perm_recursive "/data/local/tmp" 2000 2000 0755 0644 u:r:shell:s0

# ui_print "- Setting permissions /sdcard 0 0 0755 0644"
# set_perm_recursive "/sdcard" 2000 2000 0755 0644 u:r:shell:s0

# ui_print "- Setting permissions /sdcard 0 0 0755 0755"
# set_perm_recursive "/sdcard" 0 0 0755 0755
