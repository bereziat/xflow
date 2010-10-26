# (c) 2004 D.Béréziat
# Facile de créer à la main un fichier XFLOW.
# Exemple 2: convertir deux fichiers (u,v) en un fichier XFLOW
#
# $Id: uv2vel.sh,v 1.1 2006/10/04 08:36:56 bereziat Exp $

inputu=$1
inputv=$2
output=$3

# Création image XFLOW
raz `par $inputu -x -y -z -f -o` -v 2 > $output

# Copie au bon endroit des deux composantes u et v
melg $outputu $output  -ivo 1
melg $outputv $output  -ivo 2

