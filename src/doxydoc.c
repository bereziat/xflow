/* This file just contains some documentation for doxygen,
 * nothing to be compiled here 
 * $Id: doxydoc.c,v 1.16 2010/10/18 19:47:42 bereziat Exp $
 */


/**
 * \mainpage Xflow
 *
 * \section intro Introduction
 * 
 * Xflow est un programme qui se d�cline en deux parties:
 * - un utilitaire de visualisation de champs de vecteur (il s'appelle xflow)
 * - une librarie de d�veloppement (libxflow) qui offre des fonctions de 
 *   lecture et d'�criture dans des images repr�sentant des champs de vecteurs.
 *   C'est ce format d'image qui est reconnu et exploit� par l'utilitaire
 *   de visualisation. En pratique, les logiciels qui calculent des champs
 *   de vecteurs (programme de flot, etc) utiliseront cette librarie pour
 *   cr�er leurs fichiers r�sultats.
 *
 * Voir aussi l'<a class="el" href="NEWS">historique</a> des versions
 * ou le fichier <a class="el" href="ChangeLog">ChangeLog</a>.
 * 
 * \section format Format des fichiers XFLOW
 *
 * Le format XFLOW est tr�s simple : il s'agit d'une image au format INRIMAGE avec
 * un codage particulier: les images sont cod�es en flottant simple pr�cision et
 * elles sont vectorielles avec deux composantes : un pixel repr�sente un vecteur
 * � deux dimensions. La premi�re composante est la composante horizontale et la
 * seconde composante est la composante verticale. De plus, l'ent�te du fichier
 * contient un num�ro de version pour identifier la version du protocole XFLOW
 * utilis�e. On peut �galement sp�cifier un nom d'image de r�f�rence qui sert
 * au visualisateur XFLOW comme image d'arri�re plan. En pratique, il s'agit de
 * l'image qui a servi au calcul du champ de vecteurs. Cette image doit avoir
 * la m�me taille et le m�me nombre de plan que l'image du champ de vecteurs.
 *
 * Historiquement, le format XFLOW �tait plus complexe : il pouvait sp�cifier
 * une zone d'int�r�t et un pas d'�chantillonnage. Cela a �t� supprim� car en pratique
 * peu utilis� et inutile. Le champ de vecteur �tait repr�sent� par deux fichiers
 * distincts et il n'y avait pas de r�f�rence explicite � une image source. Les
 * noms des fichiers �taient utilis�s pour cela. Les anciens et nouveaux formats
 * ne sont pas compatibles. Toutefois, les utilitaires vel2uv et uv2vel permettent la 
 * conversion.
 *
 * \section doc Documentation
 *   - La \link libxflow librairie de d�veloppement\endlink.
 *   - L'utilitaire \link xflow xflow\endlink de visualisation. Ce programme permet
 *     d'exporter des r�sultats pour insertion dans des documents. Il utilise les
 *     sous programmes \link vel2fig vel2fig\endlink et \link vel2mpg vel2mpg\endlink.
 *   - La commande \link velpar velpar \endlink affiche des informations d'un fichier XFLOW.
 *   - La commande \link velset velset\endlink modifie la valeur d'un pixel dans un fichier XFLOW.
 *   - La commande \link velimg velimg\endlink (re)d�finie l'image de fond d'un fichier XFLOW.
 *   - La commande \ref gradvel est un exemple simple d'utilisation de 
 *     la librairie C et le script gradvel.sh montre la g�n�ration d'un fichier XFLOW �
 *     partir de commandes INRIMAGE.  
 *   - La commande \link velstats velstats\endlink produit des statistiques sur un ou deux champs
 *     de vecteurs.
 *   - Les scripts \link velnorm velnorm\endlink, \link vel2div vel2div\endlink, 
 *     \link vel2curl vel2curl \endlink et \link vel2mpg vel2mpg\endlink sont
 *     utilis�s par la commande \link xflow xflow\endlink pour produire des figures vectorielles et des animations.
 *   - Lisez aussi le fichier <a class="el" href="../NEWS">d'historique</a>. 
 *
 *
 * \author Dominique B�r�ziat (dominique.bereziat@lip6.fr)
 * \date Mai 2013
 *
 */

/**
 * \defgroup libxflow Librairie de lecture/�criture des fichiers XFLOW.
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
 * Les fichiers visualisables sont des fichiers au format XFLOW bien-s�r mais
 * aussi des images au format INRIMAGE :
 *  - Les fichiers XFLOW sont visualis�s comme champ de vecteurs
 *  - les images INRIMAGE sont visualis�es comme image d'arri�re-plan. 
 *
 * Les fichiers XFLOW peuvent �galement comporter une r�f�rence vers une image qui 
 * sert d'arri�re-plan. La commande \c xflow utilisera
 * la premi�re image lue (que se soit dans la ligne de commande ou comme
 * r�f�rence d'un fichier XFLOW) comme image d'arri�re-plan (toutefois, on
 * peut choisir l'image d'arri�re-plan dans l'interface grapique du 
 * programme : voir la section \link pref Formulaire de pr�f�rence\endlink).
 * En pratique, il peut �tre utile de sp�cifier plusieurs image d'arri�re-plan
 * que l'on choisira depuis l'interface de \c xflow.
 *
 * On peut sp�cifier plusieurs fichiers XFLOW : \c xflow les affichera avec
 * diff�rentes couleurs; c'est utile pour comparer visuellement plusieurs champs.
 * Lorsque plusieurs fichiers sont sp�cifi�s, ils doivent avoir la m�me
 * dimension et le m�me nombre de plans.
 *
 * Si aucun fichier n'est sp�cifi�, la commande \c xflow lira dans l'entr�e standard d'unix.
 *
 * \c xflow peut �tre param�tr�e depuis sa ligne de commande (voir <tt>xflow -help</tt> pour
 * un r�capitulatif). Ces options, qui sont <tt>-sample</tt>, <tt>-scale</tt>, <tt>-tlow</tt>, 
 * <tt>-thigh</tt>, <tt>-zoom</tt>, <tt>-norma</tt> et <tt>-smooth</tt>, correspondent aux
 * r�glages graphiques de la barre d'outil principale de \c xflow (voir section suivante).
 *
 * (lire aussi l'<a class="el" href="../NEWS">historique</a>)
 *
 * \section api Interface graphique de la commande
 *
 * Le programme xflow est une interface GTK qui comporte une fen�tre principale
 * dot�e d'un menu, d'une barre d'outil qui modifie certain aspects de 
 * l'affichage, d'une zone d'affichage avec 4 type d'affichage possibles.
 * En bas de la fen�tre on trouve une zone d'information de la zone
 * point�e par la souris.
 *
 * \image html main-window.png "Fen�tre principale de XFLOW"
 *
 * \subsection menu Le menu
 * - <tt>File</tt>
 *   - <tt>open</tt> :  non fonctionnel,
 *   - <tt>export</tt> : ouvre le formulaire d'export (voir plus loin).
 *   - <tt>Clean header</tt> : retire les param�tres d'affichage dans les 
 *     ent�tes des fichiers XFLOW.
 *   - <tt>Write header</tt> : �crit les param�tres d'affichage dans les 
 *     ent�tes des fichiers XFLOW.
 * - <tt>View</tt> 
 *   - <tt>Previous frame</tt>: affiche le plan pr�c�dent.
 *   - <tt>Next frame</tt>: affiche le plan suivant.
 *   - <tt>View vectors</tt>: s�lectionne la page V qui affiche en mode 'Vectors';
 *     les vecteurs des images lues sont affich�es par dessus l'image d'arri�re plan. 
 *   - <tt>View magnetude</tt>: s�lectionne la page M qui affiche en mode 'Magnetude';
 *      la carte de la norme du champ <b>actif</b> est affich� (en plus du champ de vecteurs)
 *   - <tt>View divergence</tt>: s�lectionne la page D. Comme M mais avec la carte de divergence.
 *   - <tt>View curl</tt>: s�lectionne la page C. Comme M mais avec la carte de rotationel.
 *   - <tt>View HSV</tt>: s�lectionne la page H. Comme M mais avec la repr�sentation 
 * colorim�trique de Middlebury.
 * - <tt>Select</tt> 
 *   <BR>dans ce menu, on peut choisir l'image d'arri�re plan
 *   (si plusieurs images ont �t� lues), ainsi que le champ de vecteur "actif", 
 *   c'est-�-dire celui qui est affich� dans les pages M, D, C, et H. 
 * - <tt>Vectors</tt><br> Il permet de choisir les attributs d'affichage du champ
 *   actif (qui est choisi dans le menu <tt>Select</tt>).
 *
 * \subsection tool La barre d'outil
 *
 * La barre d'outil est constitu�e d'un ensemble de curseurs qui permettent 
 * de modifier certains param�tres d'affichage de tous les champs de vecteurs :
 *  - <tt>sample</tt> : d�termine le pas d'�chantillonage pour l'affichage
 *    des champs : un point sur <tt>sample</tt> est affich� par ligne et une 
 *    ligne sur <tt>sample</tt> ligne est affich�e. On commence toujours par
 *    la premi�re ligne et la premi�re colonne. Ainsi, si <tt>sample</tt> vaut 1,
 *    tous les points sont affich�s.
 *  - <tt>scale</tt> : la norme de chaque vecteur est multipli�e par ce coefficient.
 *  - <tt>threshold (low)</tt> : les vecteurs dont la norme est inf�rieure �
 *    ce seuil ne seront pas affich�s.
 *  - <tt>threshold (high)</tt> : les vecteurs dont la norme est sup�rieure �
 *    ce seuil ne seront pas affich�s.
 *  - <tt>zoom</tt> : facteur d'agrandissement de la zone d'affichage.
 * La barre poss�de en outre deux boutons:
 *  - <tt>normalize</tt>: les vecteurs sont normalis�s en norme. Apr�s la normalisation
 *    ils restent sensible au param�tre <tt>scale</tt>.
 *  - <tt>smooth</tt>: le champ est liss� par un moyennage local.
 *
 * \subsection disp La zone d'affichage
 *
 * La zone d'affichage est constitu�e :
 *   - � l'extr�me gauche d'un curseur vertical qui s�lectionne le plan
 *     de visualisation.
 *   - d'une barre de 5 pages s�lectionnant le type de vue :
 *      - la page 'V' affiche les champs des vecteurs superpos�e � l'image 
 *        d'arri�re-plan.
 *      - la page 'M' affiche le champ des vecteurs superpos�e � l'image
 *        des magn�tudes c'est-�-dire la norme du champ des vecteurs. Les
 *        valeurs de magn�tude varient entre 0 (le min) et 255 (le max)
 *        en niveau de gris.
 *      - la page 'D' affiche le champ des vecteurs superpos�e � l'image
 *        de divergence du champ. Les couleurs d'affichage varient entre
 *        rouge (le minimum des valeurs n�gatives) et le bleu (le maximum
 *        des valeurs positive). Les valeurs nulles sont repr�sent�es en 
 *        noir.
 *      - la page 'C' est identique � 'D' pour le rotationnel du champ des
 *        vecteurs.
 *      - la page 'H' d�finie la carte de Middlebury. La teinte code l'orientation
 *        et la saturation code la norme.
 *   - la zone image d�finie �galement des curseurs de d�placement (� sa 
 *     droite et en dessous) si elle n'est pas enti�rement visible.

 * \subsection handle Zone contextuelle
 *
 * La poign�e d�pla�able permet de faire appara�tre une zone suppl�mentaire d'affichage
 * qui d�pend du contexte:
 *   - Dans la page V, il affiche la liste des champs de vecteur lues: on peut 
 *     controler leur affichage.
 * \image html xflow-compare.png "Comparaison de champs"
 *   - Dans les pages M, D, et C, il affiche une l�gende des couleurs utilis�es
 *     dans les cartes de magn�tude, divergence et rotationel.
 *   - Dans la page H, il affiche la roue des couleurs en fonction de l'orientation
 *     et un bouton pour controler la saturation des couleurs.
 * \image html xflow-others.png "Carte de Magn�tude, divergence, rotationelle et Middlebury"
 *
 *
 * \subsection export Le formulaire d'exportation
 *
 * Invoqu� depuis le menu, il permet de g�n�rer des figures ins�rables dans
 * des rapports ou des animations pour des pr�sentations. Vous devez
 * renseigner les champs suivants :
 *   - Generic name : nom du fichier de sortie (sans extension).
 *   - Type : type de fichier de sortie. Actuellement possible :
 *       - Pdf : fichier .pdf, pour insertion dans un document pdfLaTeX.
 *       - Encapsuled postscript : fichier .eps, pour insertion dans un document LaTeX,
 *       - Postscript : fichier .ps, pour impression
 *       - Tiff : image bitmap .tif
 *       - Jpeg : image bitmap .jpg
 *       - Mpeg sequence : g�n�re une animation mpeg, un fichier .mpg.
 *       - Inrimage sequence : g�n�re une animation Inrimage, un fichier .inr.
 *       - Avi sequence : g�n�re une animation avi, un fichier .avi.
 *   - Size : taille de l'image � g�n�rer (prendre grand),
 *   - Unit : unit� de la valeur Size, centim�tre ou pouces (inches),
 *   - Width : �paisseur de trait pour le dessin des vecteurs,
 *   - Arrow : style des fl�ches. Xflow ne sait afficher que deux styles
 *             (0 et 1) mais les autres styles sont disponibles a l'export.
 *   - bouton 'keep intermediar xfig files' : un fichier interdiaire xfig est
 *     g�n�r�. On peut ensuite appel� xfig pour retoucher ce fichier (ajout de
 *     l�gendes par exemple).
 *
 *   - Jpeg quality : si le type Jpeg est choisi, r�gle le taux de compression
 *     de l'image g�n�r�,
 *   - Video bitrate : si le type AVI est choisi, r�gle le taux de compression
 *     de l'image g�n�r�,
 *   - Codec : si le type AVI est choisi, s�lectionne l'encodeur. Les choix possibles
 *     sont :
 *       - Mjpeg : Moving Jpeg (une s�quence d'image jpeg)
 *       - Mpeg 1 : comme type � Mpeg 1
 *       - Mpeg 2
 *       - Divx 3 
 *       - Divx 4 : Mpeg 4
 *
 * Remarque : \c xflow utilise la commande \c vel2fig pour g�n�rer les
 * figure et la commande \c vel2mpg pour g�n�rer les animations. De plus, INRIMAGE
 * doit �tre correctement install� sur votre syst�me.
 *
 * Pour les types eps, ps, jpeg et tiff, seule une image est g�n�r�e avec les
 * caract�ristiques suivantes :
 *  - le plan consid�r� est celui visualis� par xflow
 *  - les valeurs de facteur d'�chelle (Scale), de pas de sous-�chantillonage 
 *    (Sample) et de seuil bas et haut (Threshold low/high) de xflow sont pris en compte
 *    pour la g�n�ration de l'image,
 *  - l'image de fond est celui qui visualise xflow,
 *  - les champs de vecteurs v
 *  
 * Pour les types animation (mpg, inr et avi), tous les plans de la
 * s�quences sont utilis�s.
 */

/**
 * \page vel2mpg Commande vel2mpg
 *
 * Script qui g�n�re une s�quence Inrimage, Mpeg ou Avi � partir d'un fichier
 * Xflow. Attention : ce programme n�cessite que mpeg_encode soit install� si
 * on souhaite encoder des s�quences Mpeg ou mencoder (mplayer) si on souhaite encoder 
 * des s�quences Avi. Ce script est utilis� par xflow pour exporter des 
 * s�quences pour des d�monstrations.
 *
 * Pour plus de d�tail, tapez :
 *
 * \code
 * % vel2mpg -help
 * \endcode
 *
 */

/**
 * \page velimg Commande velimg
 *
 * Script qui d�finit, pour un fichier XFLOW, une image de fond. Cette
 * image est utilis�e par xflow et vel2fig pour la visualisation du champ.
 * Les clients xflow g�n�ralement d�finissent cette image, mais on peut
 * changer cette d�finition en utilisant cette commande.
 *
 * Pour plus de d�tails, tapez :
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
 * Pour plus de d�tails, tapez :
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
 * Pour plus de d�tails, tapez :
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
