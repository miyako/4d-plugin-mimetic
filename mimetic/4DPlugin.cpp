/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : MIME
 #	author : miyako
 #	2017/06/20
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		
		switch(pProcNum)
		{
				// --- MIME
				
			case 1 :
//				PA_RunInMainProcess((PA_RunInMainProcessProcPtr)MIME_PARSE_MESSAGE, params);
				MIME_PARSE_MESSAGE(params);
				break;
				
			case 2 :
				MIME_Create_message(params);
				break;
				
		}
		
	}
	catch(...)
	{
		
	}
}

using namespace std;
using namespace mimetic;

#pragma mark JSON

JSONNODE *json_parse_text_param(C_TEXT &t)
{
	wstring u32;
	
#if VERSIONWIN
	u32 = wstring((wchar_t *)t.getUTF16StringPtr());
#else
	
	uint32_t dataSize = (t.getUTF16Length() * sizeof(wchar_t))+ sizeof(wchar_t);
	vector<char> buf(dataSize);
	
	PA_ConvertCharsetToCharset((char *)t.getUTF16StringPtr(),
														 t.getUTF16Length() * sizeof(PA_Unichar),
														 eVTC_UTF_16,
														 (char *)&buf[0],
														 dataSize,
														 eVTC_UTF_32);
	
	u32 = wstring((wchar_t *)&buf[0]);
#endif
	return json_parse((json_const json_char *)u32.c_str());
}

// ------------------------------------- MIME -------------------------------------

void json_conv(const char *value, wstring &u32)
{
	if(value)
	{
		C_TEXT t;
		CUTF8String u8;
		
		u8 = (const uint8_t *)value;
		t.setUTF8String(&u8);
		
#if VERSIONWIN
		u32 = wstring((wchar_t *)t.getUTF16StringPtr());
#else
		
		uint32_t dataSize = (t.getUTF16Length() * sizeof(wchar_t))+ sizeof(wchar_t);
		vector<char> buf(dataSize);
		
		PA_ConvertCharsetToCharset((char *)t.getUTF16StringPtr(),
															 t.getUTF16Length() * sizeof(PA_Unichar),
															 eVTC_UTF_16,
															 (char *)&buf[0],
															 dataSize,
															 eVTC_UTF_32);
		
		u32 = wstring((wchar_t *)&buf[0]);
#endif
	}else
	{
		u32 = L"";
	}
	
}

void json_set_text(JSONNODE *n, const wchar_t *name, string &value)
{
	if((n) && (name))
	{
		wstring w32;
		json_conv(value.c_str(), w32);
		json_push_back(n, json_new_a(name, w32.c_str()));
	}
}

void json_set_object(JSONNODE *n, const wchar_t *name, JSONNODE *o)
{
	json_set_name(o, (json_const json_char *)name);
	json_push_back(n, o);
}

void json_set_text_param(JSONNODE *n, C_TEXT &t)
{
	if(n)
	{
		json_char *json_string = json_write_formatted(n);
		
		wstring wstr = wstring(json_string);
		
#if VERSIONWIN
		t.setUTF16String((const PA_Unichar *)wstr.c_str(), (uint32_t)wstr.length());
#else
		uint32_t dataSize = (uint32_t)((wstr.length() * sizeof(wchar_t))+ sizeof(PA_Unichar));
		vector<char> buf(dataSize);
		
		uint32_t len = PA_ConvertCharsetToCharset((char *)wstr.c_str(),
																							(PA_long32)(wstr.length() * sizeof(wchar_t)),
																							eVTC_UTF_32,
																							(char *)&buf[0],
																							dataSize,
																							eVTC_UTF_16);
		
		t.setUTF16String((const PA_Unichar *)&buf[0], len);
#endif
		
		json_free(json_string);
	}
	
}

void json_set_text(JSONNODE *n, const wchar_t *name, char *value)
{
	if(n)
	{
		std::wstring w32;
		json_conv(value, w32);
		json_push_back(n, json_new_a(name, w32.c_str()));
	}
}

void json_set_number(JSONNODE *n, const wchar_t *name, json_int_t value)
{
	if(n)
	{
		json_push_back(n, json_new_i(name, value));
	}
}

#pragma mark RFC2047

void decode_header(string &value)
{
	size_t dlen = value.size() + 2;
	if(dlen)
	{
		vector<char> buf(dlen);
		char *d = &buf[0];
		char *s = (char *)value.c_str();
		
		rfc2047_decode (d, s, dlen);
		value = d;
	}else
	{
		value = "";
	}
}

#pragma mark -

void getHeader(JSONNODE *n, const wchar_t *name, string &value)
{
	decode_header(value);
	json_set_text(n, name, value);
}

void getHeaders(JSONNODE *n, const wchar_t *name, Header &header)
{
	if((n) && (name))
	{
		if(header.size())
		{
			JSONNODE *header_array = json_new(JSON_ARRAY);
			
			for(Header::iterator hbit = header.begin(); hbit != header.end(); ++hbit)
			{
				Field hh = *hbit;
				
				string name = hh.name();
				
				if(0 == strncasecmp(name.c_str(), "from", 4)) continue;
				if(0 == strncasecmp(name.c_str(), "cc", 2)) continue;
				if(0 == strncasecmp(name.c_str(), "to", 2)) continue;
				if(0 == strncasecmp(name.c_str(), "bcc", 3)) continue;
				if(0 == strncasecmp(name.c_str(), "sender", 6)) continue;
				if(0 == strncasecmp(name.c_str(), "reply-to", 8)) continue;
				if(0 == strncasecmp(name.c_str(), "message-id", 10)) continue;
				if(0 == strncasecmp(name.c_str(), "content-id", 10)) continue;
				if(0 == strncasecmp(name.c_str(), "date", 4)) continue;
				
				string value = hh.value();
				decode_header(value);
				
				JSONNODE *item = json_new(JSON_NODE);
				
				json_set_text(item, L"name", name);
				json_set_text(item, L"value", value);
				
				json_push_back(header_array, item);
			}
			json_set_object(n, name, header_array);
		}
	}
}

void getMailbox(JSONNODE *address_array, Mailbox &address)
{
	JSONNODE *item = json_new(JSON_NODE);
	
	//raw
	json_set_text(item, L"raw_addr", (char *)address.str().c_str());//4D Japan <info-jp@4d.com>
	json_set_text(item, L"raw_name", (char *)address.label().c_str());//4D Japan
	
	//RFC2047
	string value = address.str();
	decode_header(value);
	json_set_text(item, L"addr", (char *)value.c_str());
	value = address.label();
	decode_header(value);
	json_set_text(item, L"name", (char *)value.c_str());
	
	json_set_text(item, L"mailbox", (char *)address.mailbox().c_str());
	json_set_text(item, L"domain", (char *)address.domain().c_str());
	json_set_text(item, L"sourceroute", (char *)address.sourceroute().c_str());
	
	json_push_back(address_array, item);
}

void getAddress(JSONNODE *address_array, Address &address)
{
	JSONNODE *item = json_new(JSON_NODE);
	
	if(address.isGroup())
	{
		//raw
		json_set_text(item, L"raw_group", (char *)address.group().str().c_str());
		json_set_text(item, L"raw_group_name", (char *)address.group().name().c_str());
		
		//RFC2047
		string value = address.group().str();
		decode_header(value);
		json_set_text(item, L"raw_group", (char *)value.c_str());
		value = address.group().name();
		decode_header(value);
		json_set_text(item, L"group_name", (char *)value.c_str());
	}else
	{
		//raw
		json_set_text(item, L"raw_addr", (char *)address.mailbox().str().c_str());//4D Japan <info-jp@4d.com>
		json_set_text(item, L"raw_name", (char *)address.mailbox().label().c_str());//4D Japan
		
		//RFC2047
		string value = address.mailbox().str();
		decode_header(value);
		json_set_text(item, L"addr", (char *)value.c_str());
		value = address.mailbox().label();
		decode_header(value);
		json_set_text(item, L"name", (char *)value.c_str());
		
		json_set_text(item, L"domain", (char *)address.mailbox().domain().c_str());
		json_set_text(item, L"label", (char *)address.mailbox().label().c_str());
		json_set_text(item, L"sourceroute", (char *)address.mailbox().sourceroute().c_str());
	}
	
	json_push_back(address_array, item);
}

//to, cc, bcc, reply_to
void getAddress(JSONNODE *n, const wchar_t *name, AddressList &addresses)
{
	if((n) && (name))
	{
		if(addresses.size())
		{
			JSONNODE *address_array = json_new(JSON_ARRAY);
			
			for(AddressList::const_iterator abit = addresses.begin(); abit != addresses.end(); ++abit)
			{
				Address address = *abit;
				
				getAddress(address_array, address);
			}
			json_set_object(n, name, address_array);
		}
	}
}

//from
void getAddress(JSONNODE *n, const wchar_t *name, MailboxList &addresses)
{
	if((n) && (name))
	{
		if(addresses.size())
		{
			JSONNODE *address_array = json_new(JSON_ARRAY);
			
			for(MailboxList::const_iterator abit = addresses.begin(); abit != addresses.end(); ++abit)
			{
				Mailbox address = *abit;

				getMailbox(address_array, address);
			}
			json_set_object(n, name, address_array);
		}
	}
}

//sender
void getAddress(JSONNODE *n, const wchar_t *name, Mailbox &mailbox)
{
	if((n) && (name))
	{
		Mailbox address = mailbox;
		
		if(address.mailbox().length())
		{
			JSONNODE *address_array = json_new(JSON_ARRAY);
			
			getMailbox(address_array, address);
			
			json_set_object(n, name, address_array);
		}
	}
}

#pragma mark MIME

void getBlob(JSONNODE *body_array, Body &body, Header &header, PA_Variable *array_blob)
{
	JSONNODE *item = json_new(JSON_NODE);
	
	basic_stringbuf<unsigned char> buf;
	basic_ostream<unsigned char> stream(&buf);
	ostream_iterator<unsigned char, unsigned char> out(stream);
	
	string body_text = body.c_str();
	
	if(ContentTransferEncoding::base64 == header.contentTransferEncoding().mechanism())
	{
		Base64::Decoder decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}else if(ContentTransferEncoding::quoted_printable == header.contentTransferEncoding().mechanism())
	{
		QP::Decoder decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}else if(header.contentTransferEncoding().mechanism() == ContentTransferEncoding::sevenbit)
	{
		NullCodec decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}else
	{
		NullCodec decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}
	
	PA_long32 i = PA_GetArrayNbElements(*(array_blob));
	PA_ResizeArray(array_blob, ++i);
	PA_Variable element = PA_CreateVariable(eVK_Blob);
	PA_SetBlobVariable(&element, (void *)buf.str().c_str(), (PA_long32)buf.str().size());
	PA_SetBlobInArray(*(array_blob), i, element.uValue.fBlob);
	
	getHeaders(item, L"headers", header);
	
	json_set_number(item, L"data", i);
	
	string filename = header.contentDisposition().param("filename");
	if(filename.length())
	{
		decode_header(filename);
		json_set_text(item, L"filename", filename);
	}
	string name = header.contentType().param("name");
	if(name.length())
	{
		decode_header(name);
		json_set_text(item, L"name", name);
	}
	
	string mime_type = header.contentType().type();
	string mime_subtype = header.contentType().subtype();
	
	if(mime_subtype.length())
	{
		mime_type += "/";
		mime_type += mime_subtype;
	}
	
	getHeader(item, L"mime_type", mime_type);
	
	json_push_back(body_array, item);
}

void getText(JSONNODE *body_array, Body &body, Header &header)
{
	JSONNODE *item = json_new(JSON_NODE);
	
	string charset = header.contentType().param("charset");
	
	string body_text = body.c_str();

	basic_stringbuf<unsigned char> buf;
	basic_ostream<unsigned char> stream(&buf);
	ostream_iterator<unsigned char, unsigned char> out(stream);
	
	if(ContentTransferEncoding::base64 == header.contentTransferEncoding().mechanism())
	{
		Base64::Decoder decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}else if(ContentTransferEncoding::quoted_printable == header.contentTransferEncoding().mechanism())
	{
		QP::Decoder decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}else if(header.contentTransferEncoding().mechanism() == ContentTransferEncoding::sevenbit)
	{
		NullCodec decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}else
	{
		NullCodec decoder;
		decode(body_text.begin(), body_text.end(), decoder, out);
	}
	
	string decoded_bytes = string((char *)buf.str().c_str(), buf.str().size());
	
	
	size_t src_len = decoded_bytes.size();

	iconv_t cd;
	if(!charset.length()
		 || (0 != strncasecmp(charset.c_str(), "utf-8", charset.length())))
	{
		if((cd = iconv_open ("utf-8", charset.c_str())) != (iconv_t)(-1))
		{
			//src
			char *src = (char *)decoded_bytes.c_str();
			char *inbytes = src;
			size_t inbytesleft = src_len;
		
#if 1
			//incremental iconv
			//dst
			size_t dst_len = src_len;//start buffer size
			size_t outbytesleft = dst_len;
			vector<char> dst(dst_len);
			size_t pos = 0;
			char *outbuf = &dst.at(pos);
			
			while(iconv (cd, &inbytes, &inbytesleft, &outbuf, &outbytesleft) == (size_t)(-1))
			{
				size_t growth = dst_len - outbytesleft;
				pos += growth;
				dst_len += ICONV_BUFFER_SIZE;
				outbytesleft += ICONV_BUFFER_SIZE;
				dst.resize(dst_len);
				outbuf = &dst.at(pos);
			}
			
			body_text = string(&dst.at(0), (dst.size() - outbytesleft));
#else
			//bulk iconv
			//dst
			size_t dst_len = (src_len * 2) + 1;
			size_t outbytesleft = dst_len;
			vector<char> dst(dst_len);
			char *outbuf = &dst.at(0);
			
			if(iconv (cd, &inbytes, &inbytesleft, &outbuf, &outbytesleft) != (size_t)(-1))
			{
				body_text = string(&dst.at(0), (dst.size() - outbytesleft));
			}
#endif
			
			iconv_close (cd);
		}
	}else
	{
		body_text = decoded_bytes; //utf-8
	}
	
	getHeaders(item, L"headers", header);
	
	json_set_text(item, L"data", (char *)body_text.c_str());
	
	string mime_type = header.contentType().type();
	string mime_subtype = header.contentType().subtype();
	
	if(mime_subtype.length())
	{
		mime_type += "/";
		mime_type += mime_subtype;
	}
	
	getHeader(item, L"mime_type", mime_type);
	
	json_push_back(body_array, item);
}

#pragma mark -

void processMessage(JSONNODE *n, MimeEntity *me, PA_Variable *array_blob, unsigned int level)
{
	//top level may be body (text, html, or both) or attachment
	
	Body &body = me->body();
	Header &header = me->header();
	
	if(header.contentType().isMultipart())
	{
		MimeEntityList &parts = body.parts();
		
		for(MimeEntityList::iterator pbit = parts.begin(); pbit != parts.end(); ++pbit)
		{
			MimeEntity *part = *pbit;
			
			Body &part_body = part->body();
			Header &part_header = part->header();
			
			if(part_header.contentType().isMultipart())
			{
				processMessage(n, part, array_blob, ++level);
			}else
			{
				string content_disposition = part_header.contentDisposition().str();
				string mime_type = part_header.contentType().type();
				
				if((0 == strncasecmp(content_disposition.c_str(), "attachment", 10))||(level > 1))
				{
					//attachment or not top level
					JSONNODE *attachment_array = json_get(n, L"attachment");
					if(!attachment_array)
					{
						attachment_array = json_new(JSON_ARRAY);
						json_set_object(n, L"attachment", attachment_array);
					}
					getBlob(attachment_array, part_body, part_header, array_blob);
				}else if(0 == strncasecmp(mime_type.c_str(), "text", 4))
				{
					JSONNODE *body_array = json_get(n, L"body");
					if(!body_array)
					{
						body_array = json_new(JSON_ARRAY);
						json_set_object(n, L"body", body_array);
					}
					getText(body_array, part_body, part_header);
				}else
				{
					//not text
					JSONNODE *attachment_array = json_get(n, L"attachment");
					if(!attachment_array)
					{
						attachment_array = json_new(JSON_ARRAY);
						json_set_object(n, L"attachment", attachment_array);
					}
				}
			}
		}//for
		
	}else
	{
		string mime_type = header.contentType().type();
		if(0 == strncasecmp(mime_type.c_str(), "text", 4))
		{
			JSONNODE *body_array = json_get(n, L"body");
			if(!body_array)
			{
				body_array = json_new(JSON_ARRAY);
				json_set_object(n, L"body", body_array);
			}
			getText(body_array, body, header);
		}else
		{
			//not text
			JSONNODE *attachment_array = json_get(n, L"attachment");
			if(!attachment_array)
			{
				attachment_array = json_new(JSON_ARRAY);
				json_set_object(n, L"attachment", attachment_array);
			}
		}
	}
}

#pragma mark -

void MIME_PARSE_MESSAGE(PA_PluginParameters params)
{
	ios_base::sync_with_stdio(false);
	
	PackagePtr pParams = (PackagePtr)params->fParameters;
	PA_Variable Param3 = PA_CreateVariable(eVK_ArrayBlob);
	
	JSONNODE *json = json_new(JSON_NODE);
	JSONNODE *json_message = json_new(JSON_NODE);

	C_TEXT Param2;
	
	PA_Handle h = PA_GetBlobHandleParameter(params, 1);
	
	if(h)//	the handle could be NULL if the BLOB is empty on windows
	{
		istringstream ss(string(PA_LockHandle(h), PA_GetHandleSize(h)));
		istreambuf_iterator<char> bit(ss), eit;
		MimeEntity me(bit, eit);
		
		/* Mailbox */

		Mailbox sender = me.header().sender();
		getAddress(json_message, L"sender", sender);
		
		MailboxList from = me.header().from();
		getAddress(json_message, L"from", from);
		
		/* Address */

		AddressList to = me.header().to();
		getAddress(json_message, L"to", to);
		
		AddressList cc = me.header().cc();
		getAddress(json_message, L"cc", cc);
		
		AddressList reply_to = me.header().replyto();
		getAddress(json_message, L"reply_to", reply_to);
		
		AddressList bcc = me.header().bcc();
		getAddress(json_message, L"bcc", bcc);

		/* Content-ID */
		
		string content_id = me.header().contentId().str();
		getHeader(json_message, L"content_id", content_id);

		/* Message-ID */
		
		string message_id = me.header().messageid().str();
		getHeader(json_message, L"message_id", message_id);
		
		/* Subject */
		
		string subject = me.header().subject();
		getHeader(json_message, L"subject", subject);
		
		/* Content-Type */
		
//		string mime_type = me.header().contentType().str();
//		getHeader(json_message, L"mime_type", mime_type);//includes charset, etc

		string mime_type = me.header().contentType().type();
		string mime_subtype = me.header().contentType().subtype();
		
		if(mime_subtype.length())
		{
			mime_type += "/";
			mime_type += mime_subtype;
		}
		
		getHeader(json_message, L"mime_type", mime_type);
		
		if(me.hasField("date"))
		{
			string date = me.header().field("date").value();
			getHeader(json_message, L"date", date);
		}
		
		processMessage(json_message, &me, &Param3, 0);
		
		PA_UnlockHandle(h);
	}
	
	json_set_object(json, L"message", json_message);
	json_set_text_param(json, Param2);

	json_delete(json);
	
	PA_SetVariableParameter(params, 3, Param3, 0);
	Param2.toParamAtIndex(pParams, 2);
}

void MIME_Create_message(PA_PluginParameters params)
{
	PackagePtr pParams = (PackagePtr)params->fParameters;
	PA_Variable *data_array_p = ((PA_Variable *)pParams[1]);//Param2
	
	C_TEXT Param1;
	Param1.fromParamAtIndex(pParams, 1);
	JSONNODE *json = json_parse_text_param(Param1);
	
	if(json)
	{
		json_delete(json);
	}
	
	PA_ReturnBlob(params, NULL, 0);

}
