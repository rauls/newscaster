The project is a "datacasting network software" 
Lets first look at what the rest of the world is doing;
SCALA - http://www.scala.com/vignettes/datacasting-network-software.html  ,  http://www.scala.com/vignettes/index.html
Another article http://www.signindustry.com/led/articles/2004-11-30-DigitalSignageTransformsIn-Store.php3
http://www.signindustry.com/led/articles/2005-01-03-LB-ToysRusDigitalSignage.php3





Scope
=====

The project requires an easy to use GUI layout editor for determining the sequence or slideshow
list of files/media. This would utilize a common 3rd party image database server which might require
modifications for it to support movie (.mpg) files. The reason for the need of this server, is for
unique control by each "user" for each "sign machine". Sounds simple, but simple things can explode
with thousands of options and feature creaps.

Image Databases Contenders:

	MDID.org	C#/ASP/SQL


Alternative Server Designs
==========================

1. HTTP Server (basic raw server)
While this can be easily implemented using standard http file sharing and editing of playlist files in xml via
file sharing samba, it would be too crude and low level for the average paying customer, but probably addequate
for prototype proof of concept demos which would demonstrate the ability to change content and automatically
distribute the content to each Sign. This solution is fast and requires no extra code on the server end to be
generated. All thats required are free XML editing tools and an organized file/directory structure with secure
logins for file access to edit the files. This is best suited for a demo or a onsite server/client setup with no external servers or internet required.

2. Web Forms with HTTP server (ASP scripts on XP/W2k3,IIS)
This would entail webforms designed to talk to an SQL server which contains library lists, play lists per machine,
configuration details and news items. Other (ASP) scripts can be coded to feed/serve required data in XML format
from the SQL data. This requires basic webform design via IDEs such as Visual .NET while integrating into an
SQL server or ODBC database layer. The database isn't expected to be large, since even 1000 playlists with
500 items each would only come to 75meg at most. This setup would require the design of user management scripts,
configuration forms for options, individual sign machine control, and editing of playlists per machine based on
a global library of 'media content' that can be sourced from or added to. This approach is the most amount of work,
but would give you complete 100% control over the look/design/style. This would be more suited for long term future
versions or if you have a team of 4 to do it.







Firstly
=======

Now there is no way we can clone multimillion dollar systems in 1/20th the time with 5% of the resources,
those big name systems probably took 15-20 people months if not years to design and build.
If its that easy then others would be doing it now. Having said that, a simple designed version can be
done quickly as long as the scope/capabilities remain basic for the first version. Complex management and
unusual scheduling can be added much later once a working system is in operation rather than trying to achieve
everything in one step.

Current design calls for all clients to use an 'online live' server that would be accessible to all clients, but
only via the real internet, and not locally on site, which can be a big disadvantage and a major negative in the
clients minds. Factors such as ongoing internet costs, security, downtime are serious factors. As a business case,
it would be prudent to research potential clients opinions on this important and critical design decision. If no current system out there works in this way, then there must be a reason for it. One concern of mine is the amount
of access in frequency that a real world system would have, take for example if each SignBox talks to the server every
5 minutes to determine if it has new information. 10 machines is trivial load, but if we have 2000 machines nation
wide thats 2000 server requests per 300 second block, which again is fine for one server, but once we increase that
10 to 100 fold, we might have serious issues in scaling/slowness. But if that stage is reached then there would
be enough money to 'solve' the issue, but its still a noteable weak point in the design.



1. Server Requirements and technical details
============================================
* IIS on XP/W2k3 running with MSSQL or other with an image database manager (MDID).
* Multi user Logins with many 'slide shows' per client login.
* Single library of content or a library per user.
  updated/created locally at the server datacentre most likely or
  uploaded via the user if bandwidth is enough.
* Each user can create multiple slideshows of content where each one
  would be dedicated to each Sign Machine ID (MegaCorp/12). 
  A slide show can be a combination of video and stills.
* NewsBar details offered via a single static file called "news.xml" that can be
  edited at the server side or be sourced from Yahoo directly.
  If need be, it could be specific per user.
* each user would have its own folder on the server with sub folders for each machine.
  These folders are used to store the specific settings for each Sign.
  ie.  /content/MegaCorp/12/  
  These would be served out as read only, for users to 'edit' those files, they would use
  normal file server access via username/password, such as ftp or fileshare or any common
  3rd party server access method.
  Another method could be via a web form that would edit each field setting.

Server Imaging Database
MDID is a good candidate though would need modification to accept video files or at
least arbitary file usages, it could proove impossible if their code rejects anything
that isnt a still image, or it could be dead easy. This needs an attempt to find out, thought
looking at it so far it looks like it could be possible.

Custom Database Forms.
To do a custom database forms system, the follow database layout would be needed.
Each display has a machine id, MID=USERNAME.mid#   ie  BOB.64

USERS 		( uid,username,password )				Used to login to admin it
CONFIG 		( cid,name, data[1...20] )				Config control data
MACHINE		( mid,owner[uid],config[cid],location,description )	Used to identify each display uniquely
CONTENT_MEDIA	( cid,owner[uid],name,filename,thumbnail )
SLIDESHOW	( sid,owner[uid],machine[mid],name )
LOOPSEQUENCE	( lid,slideshow[sid],content[cid] )
NEWS		( nid,owner[uid], expire[date], news[text] )	




2. Client Sign Requirements and details
=======================================
* Widgie software running on either eXP or WinXP, this software does the hard work of displaying
  the images/video in sync together with a scroller, all done in C++/MFC. There is no way
  this could be achieved using .NET because it would require something like 3ghz to be efficiently
  smooth/fast.
* When mpeg2 MPG files are used, appropriate codecs need installing, there are many out there, otherwise
  if its AVI or MPEG1, it will play just like anything that WindowsMediaPlayer will play, we can play.
* Each sign machine has a unique ID to determine what its used for
  eg. it would be its username/password and machine number such as "MegaCoorp/12"
  if every sign machine has the same content, then they all can have the same ID
  such as "MegaCorp", this detail would be defined on the clients configuration
  files such as widgie.ini using a variable such as "id=MegaCorp".
  Since this only needs to be done once, it can be defined manually for the moment.
  Variables needed : (Username,Password,MachineNumber)

* Display video/stills in a sequence defined by the server based on username login
  The software would login as a specific user.
  Then it would access the 'playlist' as define for that 'machine number', so in this
  case it would be PlayList12.
  Therefore every playlist/slideshow should be named according to a scheme of
  {PLAYLIST}.{MACHINENUMBER} , this will allow automatic control.
  The client would request the list, which would define the 'play loop sequence', this 
  'list' would contain a list of files to actually play.

  Each list would loop and repeat, no predetermined 'flash adds' at 3:00pm for example yet.



* auto download new media that the client machine doesn't have.
  These media clips could be stored on the server in a predetermined location such as
  /Media/ for example, and would be global in nature with subfolders optionally used,
  unless its required that each library is unique to each user, therefore it would be stored
  in the serving url path of /widgie/MegaCorp/Media/
  If the media database server doesnt give direct access to the media, but only via a CGI url,
  then that is what will be used.

  Image Download : http://127.0.0.1/getimage.aspx?id=1&cid=1&format=F
  Movie Download : http://127.0.0.1/images/personal/video/[$MOVIE]
  SlideShow List :

* auto update of the executeable via triggers inside specific config.xml files.
  A new exe would be stored at /widgie/widgie_001.exe where 001 is its version number and would
  be used globally by all machines, unless again its required that each Client has its own.
  The config.xml would be independant of MDID2 and be a raw xml file served, unless work on an
  asp/html editor would be done.

* auto update news details hourly or a pre-determinde value inside config.xml or use
  news from commercial sources such as yahoo or 7news from their public RSS feeds.













3. Server Storage for Displays
=================================
   Everything on the server will be relative to "/content/" where MDID stores
   its data.
   MDID images will be retrieved via ASPX scripts.
   Each machine will have a unique "NAME" of "Display_XXXXXX" where XXXXXX are the
   last 6 digits of the ethernet mac address. This name will be used to access
   its own slideshow. On the MDID side we can store each slideshow in its own working
   'folder' for each project or location so as not to mix 100s of machines in one massive
   list.

   Images can be read in raw access via the /content/ path of
	"/content/personal/full/*"

   Video files will be stored in ;
	"/content/video/"
   These can be named any thing and have sub folders for various catagories, but must
   be referenced correctly in the MDID notes field setting.

   XML Config files will be stored in ;
	"/content/config/"
   Until we make a webform to edit this, we can stick with a static xml file.







4. MDID Changes / Integration Details
======================================

A) Ability to change visual layout or colors and graphics, since its all .aspx ,
   which is basically html, and images are there to be edited, it can all be changed.

   Files to change;
   Main Logo 	: images/mdid_logo.gif
   Company Name : images/cobrand.gif
   Tag Line 	: images/mdid_tagline.gif

   On the setup side, make sure a user exists in MDID that will be used for the Widgie client 
   software. This user also has to have read access to valid slideshows and be able to access
   those images.


B) Provide Ability to upload movies via MDID if possible. 
   If that isn't possible then we can upload directly to the /content/video folder via FTP using
   a special upload account.

	WWW/FTP Virtual Folder Location : /content/video/

   The client software will then download movies it doesnt have via http using the servers
   /content/video/{FILENAME} url location which is publicly available. No auth required.

   At the moment the logic is that if it doesn't exist it will download it, it will not attempt
   to download video that already exist. This could be changed to download new video files
   if the filesize has changed.


C) SlideShow Retrieval
   Request a playlist (slideshow) based on a unique ID which 
   we can suggest at being based of the last 6 digits of the MAC ADDR;
	"Display_112233" or "Display_Demo" if not found for demo purposes.
   
   All MDID interfacing is done via WebServices. No code modifications are needed to interface
   with MDID.

D) Image Retrieval Update based of the XML slide show list.
   Once the client software has downloaded an xml slide show , it will traverse its elements
   to figure out which slides 'images' have not been downloaded and will download them all
   to the locally stored "/Media/" folder.

   At the moment the logic is that if it doesn't exist it will download it, it will not attempt
   to download images that already exist. This could be changed to download new image files
   if the filesize has changed.


E) Display Application Setup and Control.
   The Display Application, "WIDGIE" firstly has a default config file called Widgie.ini which defines
   the server and other various settings. These all can be overided by Config/Config.xml which is
   automatically downloaded from the server. We can consider the ini file as the original backup settings.
  


F) IIS Settings.
   On Win2k3, you have to make sure you give .aspx, .asmx and .cs  rights to execute as .NET code.

