# 4d-plugin-mimetic

Parses a raw MIME message (an `.eml` file, or one message taken from an `.mbox`) held in a `BLOB` and returns its content as JSON text plus an `ARRAY BLOB` of decoded attachments. Parsing is done by [mimetic](http://www.codesink.org/mimetic_mime_library.html) 0.9.8; encoded header words ([RFC 2047](https://www.ietf.org/rfc/rfc2047.txt)) are decoded with code from [JaroMail](https://github.com/dyne/JaroMail) (originally from mutt); text bodies are converted to UTF-8 with [GNU libiconv](https://www.gnu.org/software/libiconv/); the JSON is written with [libjson](https://sourceforge.net/projects/libjson/).

Only parsing is implemented. The second command in the plugin, `MIME Create message`, does nothing.

| Command | Returns | Purpose |
|---|---|---|
| [MIME PARSE MESSAGE](#mime-parse-message) | `Text` + `ARRAY BLOB` (by reference) | Parse a MIME message into JSON and an array of attachment data |
| [MIME Create message](#mime-create-message) | `BLOB` | Not implemented |

**Platforms:** macOS (arm64 + x86_64 from source; see the build note below), Windows 64-bit (prebuilt `MIME.4DX`).

---

## Requirements & platform notes

- **macOS 11.5 or later** for a binary built from the current Xcode project.
- **Building on macOS.** `stdc/a/libjson.a` is not committed: run `scripts/build-macos-deps.sh` once before building in Xcode. It downloads the pinned libjson 7.6.1 release (checksum-verified) and compiles it as a universal libc++ archive with the committed `include/libjson` headers. The release workflow does the same, with a cache.
- **The Windows binary is the old prebuilt one.** `MIME.4dbase/Plugins/MIME.bundle/Contents/Windows64/MIME.4DX` was built from an earlier version of the source and does not contain the fixes marked *(since 14.3.0)* below.
- **Not thread-safe.** The manifest does not declare the commands thread-safe, so 4D does not run them in a preemptive process.
- **Failures are silent.** Neither command raises a 4D error. A message mimetic cannot make sense of produces a JSON object with empty or `null` properties; read [Error handling & troubleshooting](#error-handling--troubleshooting) before relying on a property being present.
- **Dates are not reliable outside UTC.** `utc_date` and `local_date` ignore the time zone written in the `Date` header (see [MIME PARSE MESSAGE](#mime-parse-message)). Use the raw `date` string when the offset matters.

---

## MIME PARSE MESSAGE

### Syntax

```
MIME PARSE MESSAGE ( mime ; json ; data )
```

| Parameter | Type | Description |
|---|---|---|
| `mime` | `BLOB` | Raw MIME message (the bytes of an `.eml` file) |
| `json` | `Text` | Receives the parsed message as JSON text |
| `data` | `ARRAY BLOB` | Receives the decoded content of each attachment, in the order listed in `attachments` |
| Result | — | No function result; `json` and `data` are output parameters |

All three parameters are mandatory.

### Description

`MIME PARSE MESSAGE` parses `mime`, writes a JSON document to `json`, and replaces the content of the `data` array. The JSON is pretty-printed and always has a single root property, `message`. Pass it to `JSON Parse` to get an object.

`data` is replaced by a new array with one element per attachment. Each attachment object in the JSON carries the element number of its content in `data` (1-based). *(since 14.3.0)* The array previously held by the variable you pass is released; older builds leaked it on each call.

If `mime` is an empty `BLOB`, `json` is `{"message":{"attachments":[],"body":[]}}` (pretty-printed) and `data` is empty.

#### The message object

| Property | Type | Description |
|---|---|---|
| `sender` | `Collection` of [mailbox](#mailbox-object) | The `Sender` header: 0 or 1 element |
| `from` | `Collection` of [mailbox](#mailbox-object) | The `From` header; empty if absent |
| `to` | `Collection` of [address](#address-object) | The `To` header; empty if absent |
| `cc` | `Collection` of [address](#address-object) | The `Cc` header; empty if absent |
| `reply_to` | `Collection` of [address](#address-object) | The `Reply-To` header; empty if absent |
| `bcc` | `Collection` of [address](#address-object) | The `Bcc` header; empty if absent |
| `content_id` | `Text` | The `Content-ID` header, `""` if absent |
| `message_id` | `Text` | The `Message-ID` header, `""` if absent |
| `subject` | `Text` | The `Subject` header, with encoded words decoded |
| `media_type` | `Text` | Top-level `Content-Type` type (`multipart`, `text`, …); `null` when mimetic reports no type |
| `media_subtype` | `Text` | `Content-Type` subtype (`mixed`, `plain`, …); `null` if absent |
| `mime_type` | `Text` | `media_type/media_subtype`; absent when there is no type |
| `date` | `Text` | The `Date` header as written (RFC 822 format); `null` if there is no `Date` header |
| `utc_date` | `Text` | `YYYY-MM-DDTHH:MM:SSZ`; present only when the `Date` header could be parsed. See the caveat below |
| `local_date` | `Text` | `YYYY-MM-DDTHH:MM:SS±hhmm`; present only when the `Date` header could be parsed. See the caveat below |
| `body` | `Collection` of [part](#part-object) | Text parts (plain text, HTML, …) |
| `attachments` | `Collection` of [part](#part-object) | Attachments; their content is in `data` |

There is no `headers` property on the message itself; only parts carry one.

**Date caveat.** `utc_date` is the date and time written in the header with `Z` appended: the offset (`+0900`, `EST`, …) is parsed but not applied. `local_date` converts that value from UTC to the machine's time zone, so it is off by the header's offset too. Both are correct only for headers written in UTC (`+0000`, `GMT`). **On Windows**, the offset in `local_date` is computed by different code and is wrong for zones that are not a whole number of hours.

#### Mailbox object

Used by `sender` and `from`.

| Property | Type | Description |
|---|---|---|
| `raw_addr` | `Text` | The whole mailbox as written, e.g. `4D Japan <info-jp@4d.com>` |
| `raw_name` | `Text` | The display name as written (may contain encoded words) |
| `addr` | `Text` | `raw_addr` with encoded words decoded |
| `name` | `Text` | `raw_name` with encoded words decoded |
| `mailbox` | `Text` | Local part of the address (before `@`) |
| `domain` | `Text` | Domain of the address |
| `sourceroute` | `Text` | Source route, usually `""` |

#### Address object

Used by `to`, `cc`, `bcc` and `reply_to`. A single address has:

| Property | Type | Description |
|---|---|---|
| `raw_addr` | `Text` | The whole mailbox as written |
| `raw_name` | `Text` | The display name as written |
| `addr` | `Text` | `raw_addr` with encoded words decoded |
| `name` | `Text` | `raw_name` with encoded words decoded |
| `domain` | `Text` | Domain of the address |
| `label` | `Text` | The display name as written (same as `raw_name`) |
| `sourceroute` | `Text` | Source route, usually `""` |

Note the differences from the mailbox object: there is no `mailbox` property, and there is an extra `label`.

A group (`undisclosed-recipients:;`, `Team: a@x, b@y;`) has `raw_group`, `raw_group_name` and `group_name` instead. The JSON text contains `raw_group` twice (raw, then decoded); which of the two 4D keeps after `JSON Parse` was not verified. The members of a group are not listed.

#### Part object

Every part has:

| Property | Type | Description |
|---|---|---|
| `headers` | `Collection` | The part's headers as `{"name": …, "value": …}` objects, values decoded. Headers whose name **starts with** `from`, `cc`, `to`, `bcc`, `sender`, `reply-to`, `message-id`, `content-id` or `date` (case-insensitive) are left out |
| `media_type` | `Text` | `Content-Type` type; absent when the part has no type |
| `media_subtype` | `Text` | `Content-Type` subtype, or `null` |
| `mime_type` | `Text` | `media_type/media_subtype`; `null` when the part has no type |
| `content_id` | `Text` | `Content-ID`, `""` if absent (use it to resolve `cid:` references in HTML) |
| `content_encoding` | `Text` | `Content-Transfer-Encoding` as written (`base64`, `quoted-printable`, `7bit`, …) |

A part in `body` also has:

| Property | Type | Description |
|---|---|---|
| `data` | `Text` | The decoded text, converted to UTF-8 from the part's `charset` |

A part in `attachments` also has:

| Property | Type | Description |
|---|---|---|
| `data` | `Number` | Element number of the decoded content in the `data` array |
| `filename` | `Text` | `filename` parameter of `Content-Disposition`, decoded; `null` if absent |
| `name` | `Text` | `name` parameter of `Content-Type`, decoded; `null` if absent |

#### How parts are sorted

`base64` and `quoted-printable` content is decoded; anything else is taken as is. Then, walking the message:

- A message that is not multipart goes to `body` if it is `text/*` or has no type. Otherwise it is **dropped**: it appears in neither `body` nor `attachments`.
- In a multipart message, a part goes to `attachments` if its `Content-Disposition` is `attachment`, or if it is nested deep enough (see [Error handling & troubleshooting](#error-handling--troubleshooting)). Otherwise a `text/*` part goes to `body`, and any other part is **dropped**. For example, an image sent inline (`Content-Disposition: inline`) directly inside the top-level `multipart/related` is not returned.

Text conversion:

- `charset=utf-8`: the bytes are used as is.
- Another charset: converted with libiconv. *(since 14.3.0)* A byte that is invalid in that charset becomes `?`, and an incomplete character at the very end is dropped. Older builds froze 4D on such input, and could garble non-UTF-8 bodies longer than about 2 KB.
- No `charset` parameter: converted from libiconv's default (the process locale's charset); not verified inside 4D.
- A charset libiconv does not know: `data` contains the part **still transfer-encoded** (e.g. the raw base64 text).

Header words in a charset other than UTF-8 (`=?iso-2022-jp?B?…?=`) are converted on macOS. **On Windows**, the source does not enable that conversion, so such words come back with unconverted bytes.

### Example

The test database (`MIME.4dbase`, method `Method1`) loads `Resources/test.eml` and calls the command. The method is stored in the binary structure, so its opening lines are reconstructed here rather than quoted verbatim:

```4d
C_BLOB($mime)
C_TEXT($mime_json)
ARRAY BLOB($mime_blobs;0)

DOCUMENT TO BLOB(Get 4D folder(Current resources folder)+"test.eml";$mime)
MIME PARSE MESSAGE($mime;$mime_json;$mime_blobs)
```

Reading the result with object notation (4D v18 R3 or later for `var`):

```4d
var $mime : Blob
var $json : Text
var $message; $part : Object
ARRAY BLOB($data;0)

DOCUMENT TO BLOB(Get 4D folder(Current resources folder)+"test.eml";$mime)
MIME PARSE MESSAGE($mime;$json;$data)

$message:=JSON Parse($json).message

If ($message.from.length>0)
	ALERT($message.from[0].name+" <"+$message.from[0].mailbox+"@"+$message.from[0].domain+">")
End if 
ALERT($message.subject)

For each ($part;$message.body)
	Case of 
		: ($part.mime_type="text/plain")
			// $part.data is the plain text body
		: ($part.mime_type="text/html")
			// $part.data is the HTML body
	End case 
End for each 
```

Saving every attachment in the database folder, using the element number stored in `data`:

```4d
var $attachment : Object
var $filename : Text

For each ($attachment;$message.attachments)
	$filename:=""
	If ($attachment.filename#Null)
		$filename:=$attachment.filename
	Else 
		If ($attachment.name#Null)
			$filename:=$attachment.name
		End if 
	End if 
	If ($filename#"")
		BLOB TO DOCUMENT(Get 4D folder(Database folder)+$filename;$data{$attachment.data})
	End if 
End for each 
```

`filename` comes from the message: sanitise it (path separators, `..`) before writing a file with it.

---

## MIME Create message

### Syntax

```
MIME Create message ( json ; data ) → BLOB
```

| Parameter | Type | Description |
|---|---|---|
| `json` | `Text` | Intended: a message description in the format returned by [MIME PARSE MESSAGE](#mime-parse-message) |
| `data` | `ARRAY BLOB` | Intended: attachment content |
| Result | `BLOB` | Nothing is returned |

### Description

The command is declared in the manifest, but its implementation is empty: it reads no parameter and returns nothing. Don't use it.

---

## Error handling & troubleshooting

- **No 4D error is ever raised.** Check the content of the result instead: an empty `body` and `attachments`, a `null` `media_type`, an empty `subject`. *(since 14.3.0)* If mimetic throws while parsing, the parameters still receive what was parsed up to that point; older builds left them unchanged.
- **A part is missing.** Non-text parts that are not marked `Content-Disposition: attachment` and are not nested deeply are dropped, and so is a single-part message that is not text. See [How parts are sorted](#how-parts-are-sorted).
- **A text part shows up in attachments.** Parts inside a multipart nested two levels below the top are always treated as attachments, text included. The nesting counter is also not reset between sibling multiparts, so the parts of the second nested multipart of a message are counted one level deeper than those of the first. Such text is in the `data` array, not converted to UTF-8.
- **`data` of a text part is base64 or quoted-printable text.** Its charset is not one libiconv knows; decode it yourself from the raw message.
- **`?` characters in a text body.** The bytes were invalid in the declared charset *(since 14.3.0; older builds froze 4D instead)*.
- **Wrong time in `utc_date` / `local_date`.** They ignore the header's zone offset; use `date`.
- **Memory grows with every call (older builds).** Builds before 14.3.0 never released the array previously held by the `data` variable, nor an empty BLOB handle per attachment. Fixed *(since 14.3.0)*.
- **4D quits when memory runs out.** The RFC 2047 helper calls `exit()` when `malloc` fails, which ends the 4D process instead of failing the command.
- **The macOS build fails with `library 'json' not found` (or `no such file` on `libjson.a`).** Run `scripts/build-macos-deps.sh` first; see [Requirements & platform notes](#requirements--platform-notes).

---

## Quick reference

```4d
ARRAY BLOB($data;0)
DOCUMENT TO BLOB($path;$mime)
MIME PARSE MESSAGE($mime;$json;$data)
$message:=JSON Parse($json).message
// $message.subject, .from[0].addr, .to, .date
// $message.body[i].mime_type / .data (text)
// $message.attachments[i].filename / .mime_type; content in $data{$message.attachments[i].data}
```
