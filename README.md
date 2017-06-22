# 4d-plugin-mimetic

Another MIME parse based in [mimetic](http://www.codesink.org/mimetic_mime_library.html).

Using some coee from [JaroMail](https://github.com/dyne/JaroMail) for [RFC2047](https://www.ietf.org/rfc/rfc2047.txt).

Using [libjson](https://sourceforge.net/projects/libjson/) for [JSON](http://www.json.org).

Using [iconv](https://www.gnu.org/software/libiconv/).

## Features

Only the parsing of MIME is implemented. Creating is not implemented yet.

Small (``6.4 MB`` for 4 platforms) compared to [GMIME](https://github.com/miyako/4d-plugin-gmime) implementation (``45.3 MB``).

### Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|

### Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" />

## Syntax

```
MIME PARSE MESSAGE (mime;json;data)
```

Parameter|Type|Description
------------|------------|----
mime|BLOB|Raw MIME data (``.eml`` or ``.mbox``)
json|TEXT|The parsed MIME content
data|ARRAY BLOB|Raw data (images, attachments) linked to the MIME content

### The parsed MIME content

* Properties of the root ``message`` object

``from`` : ``address`` array (optional) exactly ``1`` element, if any

``cc`` : ``address`` array (optional)

``to`` : ``address`` array (optional)

``bcc`` : ``address`` array (optional)

``sender`` : ``address`` array (optional) exactly ``1`` element, if any

``reply_to`` : ``address`` array (optional)

``date`` : ``string`` RFC not ISO (optional)

``content_id`` : ``string``

``message_id`` : ``string``

``headers`` : ``header`` array excludes ``from``, ``cc``, ``to``, ``bcc``, ``sender``, ``reply-to``, ``message-id``, ``content-id``, ``date``

``subject`` : ``string``

``body`` : ``part`` array

``attachments`` : ``part`` array

---

* Properties of an ``address`` object

``raw_addr`` : ``string``

``raw_name`` : ``string``

``addr`` : ``string``

``name`` : ``string``

``mailbox`` : ``string``

``domain`` : ``string``

``sourceroute`` : ``string``

alternatively

``raw_group`` : ``string``

``raw_group_name`` : ``string``

``group`` : ``string``

``group_name`` : ``string``

---

* Properties of a ``header`` object

``name`` : ``string``

``value`` : ``string``

---

* Properties of a ``part`` object

in case of text

``mime_type`` : ``string``

``data`` : ``string`` for ``text/*``, ``number`` for other types 

additionally, in case of blob

``filename`` : ``string``

``name`` : ``string``
