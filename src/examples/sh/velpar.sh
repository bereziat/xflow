#! /bin/bash
#
# XFLOW 
# (c) 2004 by Dominique Bereziat, INRIA, LIP6
#
# Display information about a XFLOW data
# $Id: velpar.sh,v 1.1 2006/10/04 08:36:56 bereziat Exp $

# Check if input is Inrimage
err=O

while [ $# -ne 0 ]
do
  args_bak=$@
  ifs_bak=$IFS

  if par $1 > /dev/null
  then
      input=$1   
      # Check is input is a valid XFLOW image
      IFS=" "
      set -- `par -v -f $input`
      v=$2
      f=$3    
      if [ $v = "2" -a $f = '-r' ]
      then        
          # Read optional variables in image header 
	  case $input in
	      *.gz)		  
		  IFS="="
		  set -- `zcat $input | head -256c | grep '^XFLOW_VERSION'`
		  version=${2-"<unset>"}
		  set -- `zcat $input | head -256c | grep '^XFLOW_IMAGE'`
		  image=${2-"<unset>"}
		  ;;	    
	      *)
		  IFS="="
		  set -- `head -256c $input | grep '^XFLOW_VERSION'`
		  version=${2-"<unset>"}
		  set -- `head -256c $input | grep '^XFLOW_IMAGE'`
		  image=${2-"<unset>"}		
		  ;;
	  esac	
	  echo `basename $input` "  " `par -x -y -z $input` " -img $image -version $version"
	  err=0
      else
	  echo "Error : $input is not a valid XFLOW data ($v $f)"
	  err=1
      fi
      #      exit 0
  else
      echo "Error : $1 is not an INRIMAGE image"
      err=1
      #  exit 1      
  fi
  IFS=$ifs_bak
  set -- $args_bak
  shift
done


if [ $err = 0 ]
then
    exit 0
else
    exit 1
fi
