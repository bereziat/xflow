#! /bin/bash


# CIM ne marche pas avec les images vectorielles

case $1 in
    -help)
	cat <<EOF
Usage: xflset [options] file
Modify a XFLOW file. Options are :
        -blank infile: create a blank file
        -set x y [z] u v : set pixel x y [z] to value (u,v)
        -name infile: define the background image name
EOF
	;;
    -blank)
	tty=`tty`
	raz `par $2 -x -y -z ` -v 2 -r | ifmt -key="XFLOW_IMAGE=$2" -cp > ${3-$tty}	
	;;
    -set)
	case $# in
	    6)
		echo "$4 $5" | cim -idx 1 -idy 1 -ix $2 -iy $3 $6
		;;
	    7)
		echo "$5 $6" | cim -idx 1 -idy 1 -idz 1 -ix $2 -iy $3 -iz $4 $7
		;;
	esac

	;;
    -name)
	ifmt -key="XFLOW_IMAGE=$2" $3
	;;
    *)
	echo "Option $1 non reconnue"	
	;;
esac

