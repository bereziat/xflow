#! /bin/sh

# (c) 2004 D.Béréziat
# Facile de créer à la main un fichier XFLOW.
# En complément, on utilise la commande xflset.
#
# $Id: gradvel.sh,v 1.1 2006/10/04 08:36:56 bereziat Exp $

# Usage gradvel.sh image gradient [options recfilters]

input=$1
output=$2
shift 2
param=${*--a 1 -cont 10}

# Calcul des gradients
recfilters $input /tmp/$output.dx $param -x 1
recfilters $input /tmp/$output.dy $param -y 1 

# Création image XFLOW
raz `par /tmp/$output.dx -x -y -z -f -o` -v 2 > $output

# Copie au bon endroit des deux composantes de gradient
melg /tmp/$output.dx $output  -ivo 1
melg /tmp/$output.dy $output  -ivo 2

# Effacement fichiers intermédiaires
rm  /tmp/$output.d?

# On donne l'image de référence (optionnel)
velimg $output $input
