/** 
 *
 * @mainpage
 *
 
 * @section Installation
   \htmlonly
   <b>Step 1</b>: Download and Install <a href=http://homes.gersteinlab.org/people/lh372/SOFT/bios/index.html>BIOS</a>.<br><br>
   <b>Step 2</b>: Download <a href="http://hgwdev.cse.ucsc.edu/~kent/exe/linux/blatSuite.34.zip">BlatSuite</a> - BLAT and a collection of utility programs. Note these executables have to be part of the PATH. <br><br>
   <b>Step 3</b>: Compiling the MRF components: make<br>
   <br><br>
   * \endhtmlonly
 
 * @section format File Format
 * \htmlonly
   The MRF flat file consists of three components:
   <ol>
   <li><b>Comment lines</b>. Comment lines are optional and start with a '#' character.</li>
   <li><b>Header line</b>. The header line is required and specifies the type of each column.</li>
   <li><b>Mapped reads</b>. Each read (single-end or paired-end) is represented by on line.</li>
   </ol>

   <br><br>
   <b>Required column</b>: <br>
   <ul>
   <li>AlignmentBlocks, each alignment block must contain the following attributes: TargetName:Strand:TargetStart:TargetEnd:QueryStart:QueryEnd</li>
   </ul> 

   <br><br>
   <b>Optional columns</b>: <br>
   <ul>
   <li>Sequence</li>
   <li>QualityScores</li>
   <li>QueryId</li>
   </ul> 

   <br><br>
   <b>Example file</b>:<br><br>
   # Comments<br>
   # Required field: Blocks [TargetName:Strand:TargetStart:TargetEnd:QueryStart:QueryEnd]<br>
   # Optional fields: Sequence,QualityScores,QueryId<br>
   AlignmentBlocks<br>
   chr1:+:2001:2050:1:50<br>
   chr1:+:2001:2025:1:25,chr1:+:3001:3025:26:50<br>
   chr2:-:3001:3051:1:51|chr11:+:4001:4051:1:51<br>
   chr2:-:6021:6050:1:30,chr2:-:7031:7051:31:51|chr11:+:4001:4051:1:51<br>
   contigA:+:5001:5200:1:200,contigB:-:1200:1400:200:400<br>

   <br><br>
   <b>Notes</b>:<br>
   <ul>
   <li>Paired-end reads are separated by ‘|’</li>
   <li>Alignment blocks are separated by ‘,’</li>
   <li>Features of a block are separated by ‘:’</li>
   <li>Columns are tab-delimited</li>
   <li>Columns can be arranged in any order</li>
   <li>Coordinates are one-based and closed</li>
   </ul> 

 * \endhtmlonly
 *
 */






