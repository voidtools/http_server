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
[Troubleshooting](#Troubleshooting)<br/>
[Range request](#Range-request)<br/>
[See also](#See-also)<br/>
<br/><br/><br/>



Download
--------
https://github.com/voidtools/http_server/releases

https://www.voidtools.com/forum/viewtopic.php?p=35401#http
<br/><br/><br/>



Everything HTTP Server Options:

![image](https://github.com/user-attachments/assets/c41251d7-76ca-4df3-8dfb-2019f48a25a0)
<br/><br/><br/>

An example of the Everything HTTP Server viewed from a web browser:

![image](https://github.com/user-attachments/assets/ecb63c87-0150-40c2-aa60-8e2331e241da)
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

To change the HTTP server username and password

*   In **Everything**, from the **Tools** menu, click **Options**
*   Click the **HTTP server** tab.
*   Type in a new **username** and **password**.
*   Click **OK**
<br/><br/><br/>  



Disable file downloading
------------------------

You can disable file downloading and allow clients to list results only.

To disable HTTP file downloading:

*   In **Everything**, from the **Tools** menu, click **Options**
*   Click the **HTTP server** tab.
*   Uncheck **Allow file download**.
*   Click **OK**
<br/><br/><br/>   



URL query string
----------------

Syntax:

http://localhost/?s=&o=0&c=32&j=0&i=0&w=0&p=0&r=0&m=0&path_column=0&size_column=0&date_modified_column=0&sort=name&ascending=1

key=value pairs can be omitted if not required.
 
### Keys:
<dl>
<dt id="s"><p><i>s</i></p></dt>
<dt id="q"><p><i>q</i></p></dt>
<dt id="search"><p><i>search</i></p></dt>
<dd><p>search text</p>
</dd>
<dt id="o"><p><i>o</i></p></dt>
<dt id="offset"><p><i>offset</i></p></dt>
<dd><p>display results from the nth result</p>
</dd>
<dt id="c"><p><i>c</i></p></dt>
<dt id="count"><p><i>count</i></p></dt>
<dd><p>return no more than value results</p>
</dd>
<dt id="j"><p><i>j</i></p></dt>
<dt id="json"><p><i>json</i></p></dt>
<dd><p>return results as a JSON object if value is nonzero</p>
</dd>
<dt id="i"><p><i>i</i></p></dt>
<dt id="case"><p><i>case</i></p></dt>
<dd><p>match case if value is nonzero</p>
</dd>
<dt id="w"><p><i>w</i></p></dt>
<dt id="wholeword"><p><i>wholeword</i></p></dt>
<dd><p>search whole words if value is nonzero</p>
</dd>
<dt id="p"><p><i>p</i></p></dt>
<dt id="path"><p><i>path</i></p></dt>
<dd><p>search whole paths if value is nonzero</p>
</dd>
<dt id="r"><p><i>r</i></p></dt>
<dt id="regex"><p><i>regex</i></p></dt>
<dd><p>perform a regex search if value is nonzero</p>
</dd>
<dt id="m"><p><i>m</i></p></dt>
<dt id="diacritics"><p><i>diacritics</i></p></dt>
<dd><p>match diacritics if value is nonzero</p>
</dd>
<dt id="path_column"><p><i>path_column</i></p></dt>
<dd><p>list the result's path in the json object if value is nonzero</p>
</dd>
<dt id="size_column"><p><i>size_column</i></p></dt>
<dd><p>list the result's size in the json object if value is nonzero</p>
</dd>
<dt id="date_modified_column"><p><i>date_modified_column</i></p></dt>
<dd><p>list the result's modified date in the json object if value is nonzero</p>
</dd>
<dt id="sort"><p><i>sort</i></p></dt>
<dd><p>where value can be one of the following:</p>
<table>
<tr><th class="wikinowrap">Sort name</th><th class="wikinowrap">Description</th></tr>
<tr><td class="wikinowrap">name</td><td>Sort by Name.</td></tr>
<tr><td class="wikinowrap">path</td><td>Sort by Path.</td></tr>
<tr><td class="wikinowrap">date_modified </td><td>Sort by Date Modified.</td></tr>
<tr><td class="wikinowrap">size</td><td>Sort by Size.</td></tr>
</table>
<br/>
</dd>
<dt id="ascending"><p><i>ascending</i></p></dt>
<dd><p>sort by ascending order if value is nonzero</p>
</dd>
</dl>
<br/><br/><br/>
  

Default html query strings values:

<table>
<tr><th>Key</th><th>Value</th></tr>
<tr><td>search</td><td></td></tr>
<tr><td>offset</td><td>0</td></tr>
<tr><td>count</td><td>32</td></tr>
<tr><td>json</td><td>0</td></tr>
<tr><td>case</td><td>0</td></tr>
<tr><td>wholeword</td><td>0</td></tr>
<tr><td>path</td><td>0</td></tr>
<tr><td>regex</td><td>0</td></tr>
<tr><td>diacritics</td><td>0</td></tr>
<tr><td>sort</td><td>name</td></tr>
<tr><td>ascending</td><td>1</td></tr>
</table>
<br/><br/><br/> 



Default JSON object query strings values:
<table>
<tr><th>Key</th><th>Value</th></tr>
<tr><td>search</td><td></td></tr>
<tr><td>offset</td><td>0</td></tr>
<tr><td>count</td><td>4294967295</td></tr>
<tr><td>json</td><td>1</td></tr>
<tr><td>case</td><td>0</td></tr>
<tr><td>wholeword</td><td>0</td></tr>
<tr><td>path</td><td>0</td></tr>
<tr><td>regex</td><td>0</td></tr>
<tr><td>diacritics</td><td>0</td></tr>
<tr><td>path_column</td><td>0</td></tr>
<tr><td>size_column</td><td>0</td></tr>
<tr><td>date_modified_column</td><td>0</td></tr>
<tr><td>date_created_column</td><td>0</td></tr>
<tr><td>attributes_column</td><td>0</td></tr>
<tr><td>sort</td><td>name</td></tr>
<tr><td>ascending</td><td>1</td></tr>
</table>
<br/><br/><br/> 

  

For example, search for ABC AND 123, from the starting offset of 0, displaying only the first 100 results, sorted by size descending:

http://localhost/?search=ABC+123&offset=0&count=100&sort=size&ascending=0
<br/><br/><br/> 
  

Change the default HTTP files
-----------------------------

To customize the layout of the server, the icon, folder image, file image, everything logo, sort up image, sort down image and up one folder image:

*   Create the folder</br> 
    HTTP Server<br/>
    in:<br/>
    %APPDATA%\\Everything<br/>
    If [Store settings and data in %APPDATA%\\Everything](https://www.voidtools.com/support/everything/options#store_settings_and_data_in_appdata_everything) is disabled, the HTTP Server folder must be created in the same location as your Everything.exe.</br>
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
*   In Everything, type in the following search and press ENTER:</br>
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

To disable file downloading:

*   In **_Everything_**, from the **Tools** menu, click **Options**.
*   Click the **HTTP Server** tab.
*   Uncheck **allow file download**.
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


