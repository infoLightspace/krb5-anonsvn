/*
 * src/lib/krb5/asn.1/asn1_make.c
 * 
 * Copyright 1994 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#include "asn1_make.h"

asn1_error_code asn1_make_etag(DECLARG(asn1buf *, buf),
			       DECLARG(const asn1_class , class),
			       DECLARG(const asn1_tagnum , tagnum),
			       DECLARG(const int , in_len),
			       DECLARG(int *, retlen))
     OLDDECLARG(asn1buf *, buf)
     OLDDECLARG(const asn1_class , class)
     OLDDECLARG(const asn1_tagnum , tagnum)
     OLDDECLARG(const int , in_len)
     OLDDECLARG(int *, retlen)
{
  return asn1_make_tag(buf,class,CONSTRUCTED,tagnum,in_len,retlen);
}

asn1_error_code asn1_make_tag(DECLARG(asn1buf *, buf),
			      DECLARG(const asn1_class , class),
			      DECLARG(const asn1_construction , construction),
			      DECLARG(const asn1_tagnum , tagnum),
			      DECLARG(const int , in_len),
			      DECLARG(int *, retlen))
     OLDDECLARG(asn1buf *, buf)
     OLDDECLARG(const asn1_class , class)
     OLDDECLARG(const asn1_construction , construction)
     OLDDECLARG(const asn1_tagnum , tagnum)
     OLDDECLARG(const int , in_len)
     OLDDECLARG(int *, retlen)
{
  asn1_error_code retval;
  int sumlen=0, length;

  if(tagnum > ASN1_TAGNUM_MAX) return ASN1_OVERFLOW;

  retval = asn1_make_length(buf,in_len, &length);
  if(retval) return retval;
  sumlen += length;
  retval = asn1_make_id(buf,class,construction,tagnum,&length);
  if(retval) return retval;
  sumlen += length;

  *retlen = sumlen;
  return 0;
}

asn1_error_code asn1_make_length(DECLARG(asn1buf *, buf),
				 DECLARG(const int , in_len),
				 DECLARG(int *, retlen))
     OLDDECLARG(asn1buf *, buf)
     OLDDECLARG(const int , in_len)
     OLDDECLARG(int *, retlen)
{
  asn1_error_code retval;

  if(in_len < 128){
    retval = asn1buf_insert_octet(buf, (asn1_octet)(in_len&0x7F));
    if(retval) return retval;
    *retlen = 1;
  }else{
    int in_copy=in_len, length=0;

    while(in_copy != 0){
      retval = asn1buf_insert_octet(buf, (asn1_octet)(in_copy&0xFF));
      if(retval) return retval;
      in_copy = in_copy >> 8;
      length++;
    }
    retval = asn1buf_insert_octet(buf, 0x80 | (asn1_octet)(length&0x7F));
    if(retval) return retval;
    length++;
    *retlen = length;
  }

  return 0;
}

asn1_error_code asn1_make_id(DECLARG(asn1buf *, buf),
			     DECLARG(const asn1_class , class),
			     DECLARG(const asn1_construction , construction),
			     DECLARG(const asn1_tagnum , tagnum),
			     DECLARG(int *, retlen))
     OLDDECLARG(asn1buf *, buf)
     OLDDECLARG(const asn1_class , class)
     OLDDECLARG(const asn1_construction , construction)
     OLDDECLARG(const asn1_tagnum , tagnum)
     OLDDECLARG(int *, retlen)
{
  asn1_error_code retval;

  if(tagnum < 31) {
    retval = asn1buf_insert_octet(buf, class | construction |
				       (asn1_octet)tagnum);
    if(retval) return retval;
    *retlen = 1;
  }else{
    asn1_tagnum tagcopy = tagnum;
    int length = 0;

    retval = asn1buf_insert_octet(buf, (asn1_octet)(tagcopy&0x7F));
    if(retval) return retval;
    tagcopy >>= 7;
    length++;

    for(; tagcopy != 0; tagcopy >>= 7){
      retval = asn1buf_insert_octet(buf, 0x80 | (asn1_octet)(tagcopy&0x7F));
      if(retval) return retval;
      length++;
    }

    retval = asn1buf_insert_octet(buf, class | construction | 0x1F);
    if(retval) return retval;
    length++;
    *retlen = length;
  }

  return 0;
}

asn1_error_code asn1_make_sequence(DECLARG(asn1buf *, buf),
				   DECLARG(const int , seq_len),
				   DECLARG(int *, retlen))
     OLDDECLARG(asn1buf *, buf)
     OLDDECLARG(const int , seq_len)
     OLDDECLARG(int *, retlen)
{
  asn1_error_code retval;
  int len, sum=0;

  retval = asn1_make_length(buf,seq_len,&len);
  if(retval) return retval;
  sum += len;
  retval = asn1_make_id(buf,UNIVERSAL,CONSTRUCTED,ASN1_SEQUENCE,&len);
  if(retval) return retval;
  sum += len;

  *retlen = sum;
  return 0;
}

asn1_error_code asn1_make_set(DECLARG(asn1buf *, buf),
			      DECLARG(const int , set_len),
			      DECLARG(int *, retlen))
     OLDDECLARG(asn1buf *, buf)
     OLDDECLARG(const int , set_len)
     OLDDECLARG(int *, retlen)
{
  asn1_error_code retval;
  int len, sum=0;

  retval = asn1_make_length(buf,set_len,&len);
  if(retval) return retval;
  sum += len;
  retval = asn1_make_id(buf,UNIVERSAL,CONSTRUCTED,ASN1_SET,&len);
  if(retval) return retval;
  sum += len;

  *retlen = sum;
  return 0;
}

asn1_error_code asn1_make_string(DECLARG(asn1buf *, buf),
				 DECLARG(const int , length),
				 DECLARG(const char *, string),
				 DECLARG(int *, retlen))
     OLDDECLARG(asn1buf *, buf)
     OLDDECLARG(const int , length)
     OLDDECLARG(const char *, string)
     OLDDECLARG(int *, retlen)
{
  asn1_error_code retval;

  retval = asn1buf_insert_charstring(buf,length,string);
  if(retval) return retval;

  *retlen = length;
  return 0;
}
