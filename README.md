HTTP Server Plugin for [Everything 1.5](https://www.voidtools.com/forum/viewtopic.php?f=12&t=9787)

Allow users to search and access your files from a webbrowser.

[Download](#download)<br/>
[Install Guide](#Plug-in-Installation)<br/>
[Setup Guide](#Plug-in-Setup)<br/>
[Start a HTTP Server](#Start-a-HTTP-server)<br/>
[View a HTTP server](#View-a-HTTP-server)<br/>
[Set a username and password](#Set-a-username-and-password)<br/>
[Disable file downloading](#Disable-file-downloading)<br/>
[URL query string](#URL-query-string)<br/>
[Change the default HTTP files](#Change-the-default-HTTP-files)<br/>
[Change the default HTTP server page](#Change-the-default-HTTP-server-page)<br/>
[Custom strings](#Custom-strings)<br/>
[Security](#Security)<br/>
[Disable HTTP Server](#Disable-HTTP-Server)<br/>
[Troubleshooting](#Troubleshooting)<br/>
[Range request](#Range-request)<br/>
[See also](#See-also)<br/>
<br/><br/><br/>

Everything HTTP Server Options:

![image](https://github.com/user-attachments/assets/c41251d7-76ca-4df3-8dfb-2019f48a25a0)

Everything HTTP Server in a web browser example:

![image](https://github.com/user-attachments/assets/ecb63c87-0150-40c2-aa60-8e2331e241da)
<br/><br/><br/>



Download
--------
https://github.com/voidtools/http_server/releases

https://www.voidtools.com/forum/viewtopic.php?p=35401#http
<br/><br/><br/>



Plug-in Installation
--------------------

To install a plug-in:

*   [Download a plug-in installer](#Download).
*   **Run** the plug-in installer.
*   Click **Add**.

\-or-  
  
To manually install a plug-in:

*   [Download a plug-in zip](#Download) and extract the plug-in dll to your Desktop.
*   Move the extracted plug-in dll to C:\\Program Files\\Everything\\plugins  
    where C:\\Program Files\\Everything is your Everything installation folder.
*   In **Everything**, from the **File** menu, click **Exit**.
*   **Restart Everything**.
<br/><br/><br/>



Plug-in Setup  
-------------

To manage your plug-ins:

*   In **Everything**, from the **Tools** menu, click **Options**.
*   Click the **Plug-ins** tab on the left.
<br/><br/><br/>



Start a HTTP server
-------------------

To start a HTTP server:

*   In **_Everything_**, From the **Tools** menu, click **Options**.
*   Click the **HTTP Server** tab.
*   Check **Enable HTTP server**.
*   Click **OK**.
<br/><br/><br/>



View a HTTP server
------------------

Start the HTTP server and open http://ComputerName in your web browser.<br/>
Where ComputerName is the name of the computer running the HTTP server.
<br/><br/><br/>



Set a username and password
---------------------------

Changing the username and password will take effect immediately.
<br/><br/><br/>



To change the HTTP server username and password

*   In **Everything**, from the **Tools** menu, click **Options**
*   Click the **HTTP server** tab.
*   Type in a new **username** and **password**.
*   Click **OK**
<br/><br/><br/>  



Disable file downloading
------------------------

You can disable file downloading and allow clients to list results only.
<br/><br/><br/>  



To disable HTTP file downloading:

*   In **Everything**, from the **Tools** menu, click **Options**
*   Click the **HTTP server** tab.
*   Uncheck **Allow file download**.
*   Click **OK**
<br/><br/><br/>   



URL query string
----------------

Syntax:

http://localhost/?s=&o=0&c=32&j=0&i=0&w=0&p=0&r=0&m=0&path\_column=0&size\_column=0&date\_modified\_column=0&sort=name&ascending=1

key=value pairs can be omitted if not required.
<br/><br/><br/> 

  

Keys:

_s_

_q_

_search_

search text

_o_

_offset_

display results from the nth result

_c_

_count_

return no more than value results

_j_

_json_

return results as a JSON object if value is nonzero

_i_

_case_

match case if value is nonzero

_w_

_wholeword_

search whole words if value is nonzero

_p_

_path_

search whole paths if value is nonzero

_r_

_regex_

perform a regex search if value is nonzero

_m_

_diacritics_

match diacritics if value is nonzero

_path\_column_

list the result's path in the json object if value is nonzero

_size\_column_

list the result's size in the json object if value is nonzero

_date\_modified\_column_

list the result's modified date in the json object if value is nonzero

_sort_

where value can be one of the following:

Sort name

Description

name

Sort by Name.

path

Sort by Path.

date\_modified

Sort by Date Modified.

size

Sort by Size.

  

_ascending_

sort by ascending order if value is nonzero

  

Default html query strings values:

Key

Value

search

offset

0

count

32

json

0

case

0

wholeword

0

path

0

regex

0

diacritics

0

sort

name

ascending

1

  

Default JSON object query strings values:

Key

Value

search

offset

0

count

4294967295

json

1

case

0

wholeword

0

path

0

regex

0

diacritics

0

path\_column

0

size\_column

0

date\_modified\_column

0

date\_created\_column

0

attributes\_column

0

sort

name

ascending

1
<br/><br/><br/> 

  

For example, search for ABC AND 123, from the starting offset of 0, displaying only the first 100 results, sorted by size descending:

http://localhost/?search=ABC+123&offset=0&count=100&sort=size&ascending=0
<br/><br/><br/> 
  

Change the default HTTP files
-----------------------------

You can customize the layout of the server, the icon, folder image, file image, everything logo, sort up image, sort down image and up one folder image.
<br/><br/><br/> 

  

*   Create the folder    
    HTTP Server<br/>
    in:<br/>
    %APPDATA%\\Everything<br/>
    If [Store settings and data in %APPDATA%\\Everything](https://www.voidtools.com/support/everything/options#store_settings_and_data_in_appdata_everything) is disabled, the HTTP Server folder must be created in the same location as your Everything.exe.
    
*   In **_Everything_**, from the **Tools** menu, click **Start HTTP Server**.
*   Download the following files to your HTTP Server folder:
*   [Everything-HTTP.Server.Files.zip](https://www.voidtools.com/Everything-HTTP.Server.Files.zip)
*   Edit these files in the HTTP Server folder in your "_Everything_" installation folder.
*   Everything will load these files instead of the embedded HTTP server files.
*   Hold Shift and press the reload button to force your browser to refresh.
<br/><br/><br/>   



Change the default HTTP server page
-----------------------------------

To change the default HTTP server page:

*   In **_Everything_**, from the **Tools** menu, click **Options**.
*   Click the **HTTP Server** tab.
*   Set the **Default page** to your custom page.
<br/><br/><br/>     



Custom strings
--------------

To customize the builtin HTTP server strings:

*   Download the HTTP server strings template: [http\_server\_strings.zip](https://www.voidtools.com/http_server_strings.zip)
*   Extract the http\_server\_strings.ini file to: %APPDATA%\\Everything\\HTTP server
*   Make any changes to your http\_server\_strings.ini
*   In Everything, type in the following search and press ENTER:
    /http\_server\_strings=C:\\Users\\<user>\\AppData\\Roaming\\Everything\\HTTP Server\\http\_server\_strings.ini<br/>
    where <user> is your username.</br>
*   Restart the HTTP Server:
    *   In **_Everything_**, from the **Tools** menu, click **Options**.
    *   Click the **HTTP Server** tab.
    *   Uncheck **Enable HTTP Server**.
    *   Click **Apply**.
    *   Check **Enable HTTP Server**.
    *   Click **OK**.
<br/><br/><br/>     



Security
--------

Every file and folder indexed by Everything can be searched and downloaded via the web server.
<br/><br/><br/>  

  

To disable file downloading:

*   In **_Everything_**, from the **Tools** menu, click **Options**.
*   Click the **HTTP Server** tab.
*   Uncheck **allow file download**.
<br/><br/><br/>    

See [Disable HTTP Server](#Disable-HTTP-Server) to remove the HTTP server options and prevent the HTTP server from starting.
<br/><br/><br/>  

  

Disable HTTP Server
-------------------

To disable the HTTP server:

*   Exit Everything (right click the Everything system tray icon and click Exit)
*   Open your Everything.ini in the same location as your Everything.exe
*   Change the following line:
    
    allow\_http\_server=1
    
    to:
    
    allow\_http\_server=0
    
*   Save changes and restart Everything.
<br/><br/><br/> 

  

Troubleshooting
---------------

How do I fix the Unable to start HTTP server: bind failed 10048 error?

There is already another service running on port 80.

Please try changing the Everything HTTP server port to another port.

To change the HTTP server port:

*   In **Everything**, from the **Tools** menu, click **Options**
*   Click the **HTTP server** tab.
*   Change **Listen on port** to a new port, for example 8080.
*   Click **OK**
<br/><br/><br/> 

  

Please make sure to specify this port when connecting to the web server with your web browser, for example:

http://localhost:8080
<br/><br/><br/> 

  

Range request
-------------

Everything supports range requests for streaming support.
<br/><br/><br/> 
  

See also
--------

*   [Multiple Instances](https://www.voidtools.com/support/everything/multiple_instances).
*   [HTTP Server Everything.ini options](https://www.voidtools.com/support/everything/ini#http).
*   [HTTP Server options](https://www.voidtools.com/support/everything/options#http_server).
*   https://www.voidtools.com/support/everything/http/
*   https://www.voidtools.com/forum/viewtopic.php?p=35401#http


