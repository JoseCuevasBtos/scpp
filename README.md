# scpp
Simple C Powerpoint.

This repo was inspired by [tpp](https://github.com/cbbrowne/tpp) and written in C with ncurses, so it is very likely you can compile it right away.
The program is very minimal (~350 SLOC) and is still on very early development.

## Installation
Just write on your terminal

	git clone https://github.com/JoseCuevasBtos/scpp.git
	cd scpp
	sudo make install

and *voila*! To uninstall do

	sudo make uninstall

## Usage
Just type

	scpp filename

You can also use the options `-s 3` to open at the third slide, `-s -2` to open at the second last slide and `-f` to open at the final slide.

The keys are also quite simple

	h, p, backspace ......... go previous slide
	l, n, space, enter ...... go next slide
	H ....................... go previous slide in group
	L ....................... go next slide in group
	g ....................... go first slide
	G ....................... go last slide
	q, Q .................... quit

It also reads numbers like vim, so `2h` has the same effect as pressing `h` twice, and `2g` or `2G` goes to the second slide.

![Usage demostration](demo.gif)

## Writing a presentation
The title uses

	--title: Titlename
	--subtitle: An interesting subtitle
	--author: Me
	--date: Today

To make a new slide do `--newpage`, the normal slides use markdown and other options:

	# Slide title
	## Slide subtitle

	--bcenter: Centered bold text
	This is normal text, while *this is on italics*, and **this is on boldface**.
	--icenter: Centered italics
	--center: Centered text

You can break one slide into multiple parts by using `--`:

	# Slide title
	Hey
	--
	this is on multiple slides
	--
	try it!

When utilizing `--`, slides become part of the same "group", whereas `--newpage` creates a new slide group entirely.

## Recomendations

Maybe if you dislike text presentations the program is still quite good for guitar tabs/chords.
A sample is the `guitar-score.scpp` file included which is an online tab I found for "Todas las hojas son del viento".

## To-do

- [x] Add utf8 support (`1.1`).
- [x] Command H and L to skip splitted slides (`1.2`).
- [ ] Formatting options for title information (such as `title - author`).
