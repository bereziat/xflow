# xflow - an utility to display 2D vector fields

## What is it? ##

`Xflow` is an utility devoted to display image 2D vector fields in a GTK
window. Image vectors fields are embedded in an Inrimage image data.
[http://inrimage.gforge.inria.fr](Inrimage) is then required to compile `xflow`.
`xflow` can also product graphics in various format (using `fig2dev` command
available in `transfig` or ``xfig` packages).

## Install ##
	
From [http://inrimage.gforge.inria.fr], choose a suitable version of Inrimage
for your OS. For instance, an 64-bits Ubuntu/Debian:

	$ wget https://who.rocq.inria.fr/Dominique.Bereziat/ftp/inrimage/4.6.5/inrimage_4.6.5-debian_amd64.deb
	$ sudo dpkg -i inrimage_4.6.5-debian_amd64.deb`
	$ sudo aptitude inrimage

Then, you can compile `xflow` as usual:

	$ git clone https://github.com/bereziat/xflow.git
	$ cd xflow; ./configure & make install

## Quick test ##
Here a simple example with Lena gradient computed using Sobel filter (with Inrimage):

	$ wget http://www.iis.uni-stuttgart.de/lehre/ss11/ComputerVision/lena.jpg
	$ cnvcol -bw lena.jpg >lena.inr
	$ raz >lena.grad `par lena.inr -x -y` -r -v 2
	$ echo -1 0 -1 -2 0 2 -1 0 1 | cim -x 3 -y 3 -r | conv lena.inr - -dir | melg - lena.grad
	$ echo -1 -2 -1 0 0 0 1 2 1 | cim -x 3 -y 3 -r | conv lena.inr - -dir | melg - lena.grad -ivo 2
	$ xflow lena.grad




