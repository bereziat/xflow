/* This file just contains some documentation for doxygen,
 * nothing to be compiled here 
 * $Id: doxydoc.c,v 1.16 2010/10/18 19:47:42 bereziat Exp $
 */


/**
 * \mainpage Xflow
 *
 * \section intro Introduction
 * 
 * Xflow est un programme qui se décline en deux parties:
 * - un utilitaire de visualisation de champs de vecteur (il s'appelle xflow)
 * - une librarie de développement (libxflow) qui offre des fonctions de 
 *   lecture et d'écriture dans des images représentant des champs de vecteurs.
 *   C'est ce format d'image qui est reconnu et exploité par l'utilitaire
 *   de visualisation. En pratique, les logiciels qui calculent des champs
 *   de vecteurs (programme de flot, etc) utiliseront cette librarie pour
 *   créer leurs fichiers résultats.
 *
 * Voir aussi l'<a class="el" href="NEWS">historique</a> des versions
 * ou le fichier <a class="el" href="ChangeLog">ChangeLog</a>.
 * 
 * \section format Format des fichiers XFLOW
 *
 * Le format XFLOW est très simple : il s'agit d'une image au format INRIMAGE avec
 * un codage particulier: les images sont codées en flottant simple précision et
 * elles sont vectorielles avec deux composantes : un pixel représente un vecteur
 * à deux dimensions. La première composante est la composante horizontale et la
 * seconde composante est la composante verticale. De plus, l'entête du fichier
 * contient un numéro de version pour identifier la version du protocole XFLOW
 * utilisée. On peut également spécifier un nom d'image de référence qui sert
 * au visualisateur XFLOW comme image d'arrière plan. En pratique, il s'agit de
 * l'image qui a servi au calcul du champ de vecteurs. Cette image doit avoir
 * la même taille et le même nombre de plan que l'image du champ de vecteurs.
 *
 * Historiquement, le format XFLOW était plus complexe : il pouvait spécifier
 * une zone d'intérêt et un pas d'échantillonnage. Cela a été supprimé car en pratique
 * peu utilisé et inutile. Le champ de vecteur était représenté par deux fichiers
 * distincts et il n'y avait pas de référence explicite à une image source. Les
 * noms des fichiers étaient utilisés pour cela. Les anciens et nouveaux formats
 * ne sont pas compatibles. Toutefois, les utilitaires vel2uv et uv2vel permettent la 
 * conversion.
 *
 * \section doc Documentation
 *   - La \link libxflow librairie de développement\endlink.
 *   - L'utilitaire \link xflow xflow\endlink de visualisation. Ce programme permet
 *     d'exporter des résultats pour insertion dans des documents. Il utilise les
 *     sous programmes \link vel2fig vel2fig\endlink et \link vel2mpg vel2mpg\endlink.
 *   - La commande \link velpar velpar \endlink affiche des informations d'un fichier XFLOW.
 *   - La commande \link velset velset\endlink modifie la valeur d'un pixel dans un fichier XFLOW.
 *   - La commande \link velimg velimg\endlink (re)définie l'image de fond d'un fichier XFLOW.
 *   - La commande \ref gradvel est un exemple simple d'utilisation de 
 *     la librairie C et le script gradvel.sh montre la génération d'un fichier XFLOW à
 *     partir de commandes INRIMAGE.  
 *   - La commande \link velstats velstats\endlink produit des statistiques sur un ou deux champs
 *     de vecteurs.
 *   - Les scripts \link velnorm velnorm\endlink, \link vel2div vel2div\endlink, 
 *     \link vel2curl vel2curl \endlink et \link vel2mpg vel2mpg\endlink sont
 *     utilisés par la commande \link xflow xflow\endlink pour produire des figures vectorielles et des animations.
 *   - Lisez aussi le fichier <a class="el" href="../NEWS">d'historique</a>. 
 *
 *
 * \author Dominique Béréziat (dominique.bereziat@lip6.fr)
 * \date Mai 2013
 *
 */

/**
 * \defgroup libxflow Librairie de lecture/écriture des fichiers XFLOW.
 * \brief Goulou
 */

/**
 * \page xflow Documentation de l'utilitaire XFLOW
 * 
 * \section usage Lancement de la commande
 * \code
    Usage : xflow [OPTIONS] [file1 [file2 [...]]]
 * \endcode
 *
 * Les fichiers visualisables sont des fichiers au format XFLOW bien-sûr mais
 * aussi des images au format INRIMAGE :
 *  - Les fichiers XFLOW sont visualisés comme champ de vecteurs
 *  - les images INRIMAGE sont visualisées comme image d'arrière-plan. 
 *
 * Les fichiers XFLOW peuvent également comporter une référence vers une image qui 
 * sert d'arrière-plan. La commande \c xflow utilisera
 * la première image lue (que se soit dans la ligne de commande ou comme
 * référence d'un fichier XFLOW) comme image d'arrière-plan (toutefois, on
 * peut choisir l'image d'arrière-plan dans l'interface grapique du 
 * programme : voir la section \link pref Formulaire de préférence\endlink).
 * En pratique, il peut être utile de spécifier plusieurs image d'arrière-plan
 * que l'on choisira depuis l'interface de \c xflow.
 *
 * On peut spécifier plusieurs fichiers XFLOW : \c xflow les affichera avec
 * différentes couleurs; c'est utile pour comparer visuellement plusieurs champs.
 * Lorsque plusieurs fichiers sont spécifiés, ils doivent avoir la même
 * dimension et le même nombre de plans.
 *
 * Si aucun fichier n'est spécifié, la commande \c xflow lira dans l'entrée standard d'unix.
 *
 * \c xflow peut être paramétrée depuis sa ligne de commande (voir <tt>xflow -help</tt> pour
 * un récapitulatif). Ces options, qui sont <tt>-sample</tt>, <tt>-scale</tt>, <tt>-tlow</tt>, 
 * <tt>-thigh</tt>, <tt>-zoom</tt>, <tt>-norma</tt> et <tt>-smooth</tt>, correspondent aux
 * réglages graphiques de la barre d'outil principale de \c xflow (voir section suivante).
 *
 * (lire aussi l'<a class="el" href="../NEWS">historique</a>)
 *
 * \section api Interface graphique de la commande
 *
 * Le programme xflow est une interface GTK qui comporte une fenêtre principale
 * dotée d'un menu, d'une barre d'outil qui modifie certain aspects de 
 * l'affichage, d'une zone d'affichage avec 4 type d'affichage possibles.
 * En bas de la fenêtre on trouve une zone d'information de la zone
 * pointée par la souris.
 *
 * \image html main-window.png "Fenêtre principale de XFLOW"
 *
 * \subsection menu Le menu
 * - <tt>File</tt>
 *   - <tt>open</tt> :  non fonctionnel,
 *   - <tt>export</tt> : ouvre le formulaire d'export (voir plus loin).
 *   - <tt>Clean header</tt> : retire les paramètres d'affichage dans les 
 *     entêtes des fichiers XFLOW.
 *   - <tt>Write header</tt> : écrit les paramètres d'affichage dans les 
 *     entêtes des fichiers XFLOW.
 * - <tt>View</tt> 
 *   - <tt>Previous frame</tt>: affiche le plan précédent.
 *   - <tt>Next frame</tt>: affiche le plan suivant.
 *   - <tt>View vectors</tt>: sélectionne la page V qui affiche en mode 'Vectors';
 *     les vecteurs des images lues sont affichées par dessus l'image d'arrière plan. 
 *   - <tt>View magnetude</tt>: sélectionne la page M qui affiche en mode 'Magnetude';
 *      la carte de la norme du champ <b>actif</b> est affiché (en plus du champ de vecteurs)
 *   - <tt>View divergence</tt>: sélectionne la page D. Comme M mais avec la carte de divergence.
 *   - <tt>View curl</tt>: sélectionne la page C. Comme M mais avec la carte de rotationel.
 *   - <tt>View HSV</tt>: sélectionne la page H. Comme M mais avec la représentation 
 * colorimétrique de Middlebury.
 * - <tt>Select</tt> 
 *   <BR>dans ce menu, on peut choisir l'image d'arrière plan
 *   (si plusieurs images ont été lues), ainsi que le champ de vecteur "actif", 
 *   c'est-à-dire celui qui est affiché dans les pages M, D, C, et H. 
 * - <tt>Vectors</tt><br> Il permet de choisir les attributs d'affichage du champ
 *   actif (qui est choisi dans le menu <tt>Select</tt>).
 *
 * \subsection tool La barre d'outil
 *
 * La barre d'outil est constituée d'un ensemble de curseurs qui permettent 
 * de modifier certains paramètres d'affichage de tous les champs de vecteurs :
 *  - <tt>sample</tt> : détermine le pas d'échantillonage pour l'affichage
 *    des champs : un point sur <tt>sample</tt> est affiché par ligne et une 
 *    ligne sur <tt>sample</tt> ligne est affichée. On commence toujours par
 *    la première ligne et la première colonne. Ainsi, si <tt>sample</tt> vaut 1,
 *    tous les points sont affichés.
 *  - <tt>scale</tt> : la norme de chaque vecteur est multipliée par ce coefficient.
 *  - <tt>threshold (low)</tt> : les vecteurs dont la norme est inférieure à
 *    ce seuil ne seront pas affichés.
 *  - <tt>threshold (high)</tt> : les vecteurs dont la norme est supérieure à
 *    ce seuil ne seront pas affichés.
 *  - <tt>zoom</tt> : facteur d'agrandissement de la zone d'affichage.
 * La barre possède en outre deux boutons:
 *  - <tt>normalize</tt>: les vecteurs sont normalisés en norme. Après la normalisation
 *    ils restent sensible au paramètre <tt>scale</tt>.
 *  - <tt>smooth</tt>: le champ est lissé par un moyennage local.
 *
 * \subsection disp La zone d'affichage
 *
 * La zone d'affichage est constituée :
 *   - à l'extrême gauche d'un curseur vertical qui sélectionne le plan
 *     de visualisation.
 *   - d'une barre de 5 pages sélectionnant le type de vue :
 *      - la page 'V' affiche les champs des vecteurs superposée à l'image 
 *        d'arrière-plan.
 *      - la page 'M' affiche le champ des vecteurs superposée à l'image
 *        des magnétudes c'est-à-dire la norme du champ des vecteurs. Les
 *        valeurs de magnétude varient entre 0 (le min) et 255 (le max)
 *        en niveau de gris.
 *      - la page 'D' affiche le champ des vecteurs superposée à l'image
 *        de divergence du champ. Les couleurs d'affichage varient entre
 *        rouge (le minimum des valeurs négatives) et le bleu (le maximum
 *        des valeurs positive). Les valeurs nulles sont représentées en 
 *        noir.
 *      - la page 'C' est identique à 'D' pour le rotationnel du champ des
 *        vecteurs.
 *      - la page 'H' définie la carte de Middlebury. La teinte code l'orientation
 *        et la saturation code la norme.
 *   - la zone image définie également des curseurs de déplacement (à sa 
 *     droite et en dessous) si elle n'est pas entièrement visible.

 * \subsection handle Zone contextuelle
 *
 * La poignée déplaçable permet de faire apparaître une zone supplémentaire d'affichage
 * qui dépend du contexte:
 *   - Dans la page V, il affiche la liste des champs de vecteur lues: on peut 
 *     controler leur affichage.
 * \image html xflow-compare.png "Comparaison de champs"
 *   - Dans les pages M, D, et C, il affiche une légende des couleurs utilisées
 *     dans les cartes de magnétude, divergence et rotationel.
 *   - Dans la page H, il affiche la roue des couleurs en fonction de l'orientation
 *     et un bouton pour controler la saturation des couleurs.
 * \image html xflow-others.png "Carte de Magnétude, divergence, rotationelle et Middlebury"
 *
 *
 * \subsection export Le formulaire d'exportation
 *
 * Invoqué depuis le menu, il permet de générer des figures insérables dans
 * des rapports ou des animations pour des présentations. Vous devez
 * renseigner les champs suivants :
 *   - Generic name : nom du fichier de sortie (sans extension).
 *   - Type : type de fichier de sortie. Actuellement possible :
 *       - Pdf : fichier .pdf, pour insertion dans un document pdfLaTeX.
 *       - Encapsuled postscript : fichier .eps, pour insertion dans un document LaTeX,
 *       - Postscript : fichier .ps, pour impression
 *       - Tiff : image bitmap .tif
 *       - Jpeg : image bitmap .jpg
 *       - Mpeg sequence : génère une animation mpeg, un fichier .mpg.
 *       - Inrimage sequence : génère une animation Inrimage, un fichier .inr.
 *       - Avi sequence : génère une animation avi, un fichier .avi.
 *   - Size : taille de l'image à générer (prendre grand),
 *   - Unit : unité de la valeur Size, centimètre ou pouces (inches),
 *   - Width : épaisseur de trait pour le dessin des vecteurs,
 *   - Arrow : style des flèches. Xflow ne sait afficher que deux styles
 *             (0 et 1) mais les autres styles sont disponibles a l'export.
 *   - bouton 'keep intermediar xfig files' : un fichier interdiaire xfig est
 *     généré. On peut ensuite appelé xfig pour retoucher ce fichier (ajout de
 *     légendes par exemple).
 *
 *   - Jpeg quality : si le type Jpeg est choisi, règle le taux de compression
 *     de l'image généré,
 *   - Video bitrate : si le type AVI est choisi, règle le taux de compression
 *     de l'image généré,
 *   - Codec : si le type AVI est choisi, sélectionne l'encodeur. Les choix possibles
 *     sont :
 *       - Mjpeg : Moving Jpeg (une séquence d'image jpeg)
 *       - Mpeg 1 : comme type à Mpeg 1
 *       - Mpeg 2
 *       - Divx 3 
 *       - Divx 4 : Mpeg 4
 *
 * Remarque : \c xflow utilise la commande \c vel2fig pour générer les
 * figure et la commande \c vel2mpg pour générer les animations. De plus, INRIMAGE
 * doit être correctement installé sur votre système.
 *
 * Pour les types eps, ps, jpeg et tiff, seule une image est générée avec les
 * caractéristiques suivantes :
 *  - le plan considéré est celui visualisé par xflow
 *  - les valeurs de facteur d'échelle (Scale), de pas de sous-échantillonage 
 *    (Sample) et de seuil bas et haut (Threshold low/high) de xflow sont pris en compte
 *    pour la génération de l'image,
 *  - l'image de fond est celui qui visualise xflow,
 *  - les champs de vecteurs v
 *  
 * Pour les types animation (mpg, inr et avi), tous les plans de la
 * séquences sont utilisés.
 */

/**
 * \page vel2mpg Commande vel2mpg
 *
 * Script qui génère une séquence Inrimage, Mpeg ou Avi à partir d'un fichier
 * Xflow. Attention : ce programme nécessite que mpeg_encode soit installé si
 * on souhaite encoder des séquences Mpeg ou mencoder (mplayer) si on souhaite encoder 
 * des séquences Avi. Ce script est utilisé par xflow pour exporter des 
 * séquences pour des démonstrations.
 *
 * Pour plus de détail, tapez :
 *
 * \code
 * % vel2mpg -help
 * \endcode
 *
 */

/**
 * \page velimg Commande velimg
 *
 * Script qui définit, pour un fichier XFLOW, une image de fond. Cette
 * image est utilisée par xflow et vel2fig pour la visualisation du champ.
 * Les clients xflow généralement définissent cette image, mais on peut
 * changer cette définition en utilisant cette commande.
 *
 * Pour plus de détails, tapez :
 *
 * \code
 * % velimg -help
 * \endcode
 * 
 */

/**
 * \page vel2div Commande vel2div
 *
 * Script qui ...
 *
 * Pour plus de détails, tapez :
 *
 * \code
 * % vel2div
 * \endcode
 * 
 */

/**
 * \page vel2curl Commande vel2curl
 *
 * Script qui ...
 *
 * Pour plus de détails, tapez :
 *
 * \code
 * % vel2curl
 * \endcode
 * 
 */

/**
 * \page velstats Commande velstats
 *
 * Calcule diverses statistiques sur un champ de vecteur ou
 * entre deux champs de vecteurs.
 *
 * A faire
 */
