#                mysqlcrypt.sh
# This shell script will run KMyMoney in a MySql database
# and uses the MySql dump program to maintain a copy of the data.
# Additionally, another copy is written to a backup
# directory before running KMyMoney. These backups will be deleted after a
# user-specified number of days.

# Optionally, these data files may be encrypted.
# For encryption, it is necessary that you have the 'gpg' program installed,
# and have set up a key identified by an email address (which may be a pseudo address).
# See 'man gpg' for more info. The kgpg program will help set up these keys.
# DO NOT forget the password associated with gpg; you will need it to access your data.

# Optionally, the data can be removed from the database after KMyMoney finishes, 
# and reloaded next time you run.

# It is assumed that your logon user name has the necessary database permissions.

# Please set the following variables to your requirements
MYDIR=$HOME/money # directory where the encrypted copy is to be held
BUDIR=$MYDIR/backup # directory where the backups should go;
FILE=myfin # name for the encrypted copy
DBNAME=KMyMoney # mysql database name
DROP=y # (y/n) - whether to delete info from database after running kmm
SILENT=n # if set to y, backups will be deleted silently, else you will be asked
CRYPTEMAIL=me@googlemail.com # delete this line if you don't want encrypted copies
declare -i KEEP=7 # number of days to keep backups
# end of user-changeable directives

if [ ! -d $MYDIR ] ; then
  mkdir $MYDIR;
fi
if [ ! -d $BUDIR ] ; then
  mkdir $BUDIR;
fi

if [ -z $CRYPTEMAIL ]; then
  EFILE=${FILE};
else
  EFILE=${FILE}.gpg;
fi

declare NOW=`date +%Y%m%d%H%M%S`

if [ ! -f $MYDIR/$EFILE ] ; then
  kdialog --warningcontinuecancel "A version of $FILE does not exist in $MYDIR.\nWhen KMyMoney starts, please open or create a file and use the 'File/Save as Database' function."
  if [ $? -ne 0 ]; then
    exit;
  fi
  KMMCMD=-n;
else
  KMMCMD=sql://$USER@localhost/$DBNAME?driver=QMYSQL3
  # backup file
  cp $MYDIR/$EFILE $BUDIR/$NOW$EFILE
  LOAD=y
  mysql -e "use $DBNAME;" 2>/dev/null
  if [ $? -eq 0 ]; then
    kdialog --warningyesno "A $DBNAME database exists. Do you wish to run with this?\n If not, the database will be reloaded from the encrypted file"
    if [ $? -eq 0 ]; then # replied yes
      LOAD=n
    fi;
  fi;
  if [ $LOAD = y ]; then
    echo "Reloading from file"
    if [ -z $CRYPTEMAIL ]; then
      mysql <$MYDIR/$EFILE;
    else
      gpg --decrypt $MYDIR/$EFILE |mysql;
    fi;
  fi;
fi
#run kmymoney
kmymoney $KMMCMD
mysqldump --databases -R $DBNAME >$MYDIR/$FILE
if [ ! -z $CRYPTEMAIL ]; then
  rm -f $MYDIR/$EFILE
  gpg -e -r $CRYPTEMAIL $MYDIR/$FILE
  rm $MYDIR/$FILE;
fi

case ${DROP:0:1} in
  y | Y) mysql -e "drop database KMyMoney;";;
esac

# delete outdated backup files
cd $BUDIR
for i in `find . -name "*${EFILE}" -ctime +${KEEP}`; do
  case ${SILENT:0:1} in
    y | Y) ANSWER=y
    ;;
    *) read -p "Delete $i?" ANSWER
    ;;
  esac
  case ${ANSWER:0:1} in
    y | Y) echo "Deleting $i!"
    rm $i
    ;;
  esac
done
