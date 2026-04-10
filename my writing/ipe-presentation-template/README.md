# Getting Started With IPE
IPE is a super cool drawing editor that can come in handy for creating scientific figures and presentations.
It is a personal favorite software within the Algorithmics Chair and students are highly encouraged to discover its many benefits themselves.

## Installation
A current version of IPE can be obtained by following the installation instructions on the [official website](https://ipe.otfried.org/).
The installation usually also includes the IpePresenter, a useful application for presenting PDF slides that you created with IPE (or any other program, but why would you do that?)

## Learing How to Use IPE

**Our IPE Tutorial**

To give you a headstart, we created an interactive IPE tutorial that will teach you all you need to know to successfully make your own figures or presentations.

You can do the tutorial by opening the file `ipe_tutorial.pdf` in IPE. 
This immediately shows you one cool property of IPE: Any PDF document that was created using IPE can be opened in it and modified again.

We suggest that you follow the contained instructions prompting you to get active yourself.
This will give you a good feel for how you can achieve what you want with IPE.
However, feel free to skip instructions that seem too simple and just refer back to the tutorial again later if you're uncertain.
In general, IPE is quite intuitive and self-explanatory and also includes a lot of tool-tips. We're confident you'll get the hang of it quickly even if you don't do the turorial!
If you have used other drawing editors before, you'll likely be familiar with some of the concepts already.

**The Official IPE Documentation**

If you ever need more detailed explanations about the specifics of IPE, the [official documentation/manual](https://otfried.github.io/ipe/) will likely help you.

Unfortunately, other online support (e.g. StackOverflow) for IPE is scarce, but odds are you won't be needing any for what you need to do.


# The AlgoKN IPE Template
This repository contains the "Algorithmics Chair of the University of Konstanz"-themed IPE template.

The main objective was to provide you with a template offering a simple, yet professional look inspired by the university's corporate design. Worry less about making a well-looking presentation by using this Prof-Storandt-approved template and focus more on the content itself!

## Highlights

The template defines and contains the following (characteristic & important) style elements and definitions:

- Slide format and dimensions (16:9, 1920x1080 pixels)

- Background including a bottom bar according to the university's corporate design and the Algorithmics Chair's logo

- Pre-defined title page

- University of Konstanz extended corporate design colors (also available in a separate style sheet `unikn_colors.isy`)

- Page titles, item dashes and underlines (`\ul{}`) in 'Seeblau'

- Command `\subheader{}`/text environment `subheadline`, as well as command `\define{}`

- `algorithm2e` Latex package for pseudocode "pre-loaded"

- Small set of sample decorations

- Extended set of definitions for font sizes, pen widths, dash styles etc. compared to default presentation style sheet

  
  

## Usage Instructions

The template is provided as 3 different files (one ending in `.ipe`, `.isy`, and `_dev.isy`, respectively).
Let us shortly point out the differences here:

  
**.ipe**
- Easiest to use (sufficient in almost all cases) is the template file ending in `.ipe`. It comes with the style file added, title page already inserted, preamble modified, and page numbers enabled.

- To use it, simply create a copy of the file and then edit the copy as you wish.

- The titlepage and bottom bar can be customized in the Latex preamble in the document properties (Ctrl + Shift + p).

**.isy**
- The file ending in `.isy` is the so called 'IPE sytle file' (similar to a CSS sheet) which defines all of the styles that constitute the given template. It is self-contained, meaning no other style sheets are required additionally to get the template.

- This file may be added to any (empty) IPE project (instead of the `basic` stylesheet, on top of the non-removable `standard` stylesheet) to gain access to all the definitions contained in it.

- The title page is contained as a Symbol, which can be accessed via `Ipelets > Symbols > use symbol`.. It may be used if desired. To do so, activate vertex snapping mode and drag it onto the page so that it aligns nicely.

- To make full use of the template and get it to work, add the following to the Latex preamble in the document properties (Ctrl + Shift + p). The defined commands can be used to customize the title page and bottom bar.

```

%Enter the information for your title page here:

\newcommand{\fulltitle}{Full Presentation Title} %Potentially break manually using \\

\newcommand{\subtitle}{Subtitle}

\newcommand{\name}{Firstname Lastname}

\newcommand{\subject}{Study Subject}

\newcommand{\displayDate}{\today}

  

%Enter the information for the bottom bar here:

\newcommand{\shorttitle}{Pres Title}

\newcommand{\bottomleft}{\displayDate}

\newcommand{\bottommiddle}{\shorttitle}

\newcommand{\bottomright}{\name}

```

- Page numbering may be enable in the document properties (Ctrl + Shift + p) if desired.

**_dev.ipe**
- The file ending in `_dev.ipe` is the one which was used to create the characteristic template background and title page. The elements that compose them are present as objects (instead of as a symbol) and can therefore be modified individually. This file may be of interest to you if you wish to tweak the design.

- To tweak the design, ungroup the elements, apply your modifications, re-group all elements belonging to the design in question (background or title page) and create a new symbol from it via `Ipelets > Symbols > create symbol`.

- For a short introduction on creating IPE templates yourself, have a look at [this mini tutorial from another source](https://olejorik.github.io/post/ipe_presentation_template/).