/* -*- Mode: C; indent-tabs-mode: nil; -*-
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the TripleDB database library.
 *
 * The Initial Developer of the Original Code is Geocast Network Systems.
 * Portions created by Geocast are
 * Copyright (C) 2000 Geocast Network Systems. All
 * Rights Reserved.
 *
 * Contributor(s): Terry Weissman <terry@mozilla.org>
 */

/* These are debugging routines for the TDB.  Don't use any of these calls
   in production code! */

extern void TDBDumpNode(TDBFileDesc* fid, TDBNode* node);
extern void TDBDumpTree(TDBFileDesc* fid, TDB* db, TDBInt32 tree);

extern TDBStatus TDBSanityCheck(TDBBase* base, TDBFileDesc* fid);


extern void TDBGetCursorStats(TDBCursor* cursor,
                              TDBInt32* hits,
                              TDBInt32* misses);


/* Create a "dot" graph file.  To view these, and learn more about them, 
   see http://www.research.att.com/~north/cgi-bin/webdot.cgi */

extern void TDBMakeDotGraph(TDB* db, const char* filename, TDBInt32 tree);
