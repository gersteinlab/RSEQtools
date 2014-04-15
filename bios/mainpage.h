/** 
 *
 * @mainpage
 *
 * @section Introduction
 * \htmlonly
   BIOS is a library entirely written in C. This library was developed to provide a framework for Bioinformatics applications. The core modules (array, format, hlrmisc, html, linesteam, and log) originate from the Roche Bioinformatics Software Objects and Services library. These modules were written by the Bioinformatics group at Roche in Basel, Switzerland. This group is headed by Dr. Clemens Broger and the core modules have been reproduced with his permission. Anyone can use or modify this library as long as proper acknowledgement is given to the author(s). The files <a href=http://homes.gersteinlab.org/people/lh372/SOFT/bios/format.txt>format.txt</a> and <a href=http://homes.gersteinlab.org/people/lh372/SOFT/bios/array.txt>array.txt</a> provide a brief overview of the core functions (namely dynamic arrays and dynamic strings).<br><br>
Other modules (bits, common, dlist, and seq) have been adapted from the BLAT source code written by Dr. James Kent at UCSC.<br><br>This library will be updated as additional modules are developed in the <a href=http://www.gersteinlab.org>Gerstein Lab</a>.<br><br>
 * \endhtmlonly
 *
 * @section Installation
 * \htmlonly
   <b>Step 1</b>: Download BIOS.<br><br>
   <b>Step 2</b>: Other requirements: <a href=http://www.gnu.org/software/gsl/>GNU Scientific Library</a><br><br>
   <b>Step 3</b>: Set the environment variables:<br>
    &nbsp - BIOINFOCONFDIR=/pathTo/bios/conf<br>
    &nbsp - BIOINFOGSLDIR=/pathTo/gsl-X.XX<br><br>
    <b>Step 4</b>: Compiling BIOS:<br>
    &nbsp;&nbsp;make<br>
    &nbsp;&nbsp;make prod<br><br><br>

   <b>Optional</b>: <a href=http://www.stack.nl/~dimitri/doxygen/>Doxygen</a> (version 1.5.7.1) is used to document the code.<br><br>

   The following environment variables are required:<br>
    - BIOINFODOXYGENBINDIR=/pathTo/doxygen-1.5.7.1/bin<br>
    - BIOINFODOXYGENDOCDIR=/pathTo/public_html/bios<br>
    - BIOINFOWEBSERVER=webServer.org<br>
   <br><br>
 * \endhtmlonly
 */

