/******************************************************************************
/ SnM_FXChainView.cpp
/ JFB TODO? now, SnM_Resources.cpp/.h would be better names..
/
/ Copyright (c) 2009-2011 Tim Payne (SWS), Jeffos
/ http://www.standingwaterstudios.com/reaper
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/ 
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/ 
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/

//JFB TODO?
// ShellExecute("edit", .. ?
// import/export slots
// combined flags for insert media (dbl click)

#include "stdafx.h"
//#include "../../WDL/projectcontext.h"
#include "SnM_Actions.h"
#include "SNM_FXChainView.h"
#ifdef _WIN32
#include "../MediaPool/DragDrop.h" //JFB: move to the trunk?
#endif


// Commands
#define AUTO_FILL_DIR_MSG			0xF001 // common cmds
#define AUTO_FILL_PRJ_MSG			0xF002
#define AUTO_FILL_DEFAULT_MSG		0xF003
#define CLEAR_SLOTS_MSG				0xF004
#define DEL_SLOTS_MSG				0xF005
#define DEL_FILES_MSG				0xF006
#define ADD_SLOT_MSG				0xF007
#define INSERT_SLOT_MSG				0xF008
#define EDIT_MSG					0xF009
#define EXPLORE_MSG					0xF00A
#define LOAD_MSG					0xF00B
#define AUTOSAVE_MSG				0xF00C
#define AUTOSAVE_DIR_MSG			0xF00D
#define AUTOSAVE_DIR_PRJ_MSG		0xF00E
#define AUTOSAVE_DIR_DEFAULT_MSG	0xF00F
#define FILTER_BY_NAME_MSG			0xF010
#define FILTER_BY_PATH_MSG			0xF011
#define FILTER_BY_COMMENT_MSG		0xF012
#define RENAME_MSG					0xF013
#define FXC_APPLY_TR_MSG			0xF020 // fx chains cmds
#define FXC_APPLY_TAKE_MSG			0xF021
#define FXC_APPLY_ALLTAKES_MSG		0xF022
#define FXC_COPY_MSG				0xF023
#define FXC_PASTE_TR_MSG			0xF024
#define FXC_PASTE_TAKE_MSG			0xF025
#define FXC_PASTE_ALLTAKES_MSG		0xF026
#define FXC_AUTOSAVE_INPUT_FX		0xF027
#define FXC_AUTOSAVE_TR_MSG			0xF028
#define FXC_AUTOSAVE_ITEM_MSG		0xF029
#define TRT_APPLY_MSG				0xF040 // track template cmds
#define TRT_APPLY_WITEMS_MSG		0xF041
#define TRT_IMPORT_MSG				0xF042
#define TRT_REPLACE_ITEMS_MSG		0xF043
#define TRT_PASTE_ITEMS_MSG			0xF044
#define TRT_AUTOSAVE_WITEMS_MSG		0xF045

#define PRJ_SELECT_LOAD_MSG			0xF050 // project template cmds
#define PRJ_SELECT_LOAD_TAB_MSG		0xF051
#define PRJ_AUTOFILL_RECENTS_MSG	0xF052
#define PRJ_LOADER_CONF_MSG			0xF053
#define PRJ_LOADER_SET_MSG			0xF054
#define PRJ_LOADER_CLEAR_MSG		0xF055
#define MED_PLAY_MSG				0xF060  // media file cmds
#define MED_LOOP_MSG				0xF061
#define MED_ADD_CURTR_MSG			0xF062
#define MED_ADD_NEWTR_MSG			0xF063
#define MED_ADD_TAKES_MSG			0xF064
#define MED_AUTOSAVE_MSG			0xF065
#ifdef _WIN32
#define THM_LOAD_MSG				0xF070  // theme file cmds
#endif

// labels for undo points and popup menu items
#define FXC_APPLY_TR_STR		"Paste (replace) FX chain to selected tracks"
#define FXCIN_APPLY_TR_STR		"Paste (replace) input FX chain to selected tracks"
#define FXC_APPLY_TAKE_STR		"Paste (replace) FX chain to selected items"
#define FXC_APPLY_ALLTAKES_STR	"Paste (replace) FX chain to selected items, all takes"
#define FXC_PASTE_TR_STR		"Paste FX chain to selected tracks"
#define FXCIN_PASTE_TR_STR		"Paste input FX chain to selected tracks"
#define FXC_PASTE_TAKE_STR		"Paste FX chain to selected items"
#define FXC_PASTE_ALLTAKES_STR	"Paste FX chain to selected items, all takes"
#define TRT_APPLY_STR			"Apply track template to selected tracks (w/o items)"
#define TRT_APPLY_WITEMS_STR	"Apply track template to selected tracks (w/ items)"
#define TRT_IMPORT_STR			"Import tracks from track template"
#define TRT_APPLY_ITEMS_STR		"Replace items of selected tracks"
#define TRT_PASTE_ITEMS_STR		"Paste items to selected tracks"
#define PRJ_SELECT_LOAD_STR		"Select/load project template"
#define PRJ_SELECT_LOAD_TAB_STR	"Select/load project templates (new tab)"
#define MED_PLAY_STR			"Play media file in selected tracks (toggle)"
#define MED_LOOP_STR			"Play/loop media file in selected tracks (toggle)"
#define MED_PLAYLOOP_STR		"Play/loop media file"
#define MED_ADD_STR				"Insert media"
#ifdef _WIN32
#define THM_LOAD_STR			"Load theme"
#endif

#define DRAGDROP_EMPTY_SLOT		">Empty<"
#define FILTER_DEFAULT_STR		"Filter"
#define AUTO_SAVE_ERR_STR		"Probable cause: no selection, cannot write file, invalid filename, empty chunk, etc..."
#define AUTO_FILL_ERR_STR		"Probable cause: all files are already present, empty/invalid directory, etc..."

enum {
  COMBOID_TYPE=2000, //JFB would be great to have _APS_NEXT_CONTROL_VALUE *always* defined
  TXTID_DBL_TYPE,
  COMBOID_DBLCLICK_TYPE,
  TXTID_DBL_TO,
  COMBOID_DBLCLICK_TO,
  BUTTONID_AUTO_SAVE
};

enum {
  FXC_AUTOSAVE_PREF_TRACK=0,
  FXC_AUTOSAVE_PREF_INPUT_FX,
  FXC_AUTOSAVE_PREF_ITEM
};


/*JFB static*/ SNM_ResourceWnd* g_pResourcesWnd = NULL;

// JFB important notes:
// all global WDL_PtrList vars used to be WDL_PtrList_DeleteOnDestroy ones but
// something weird could occur when REAPER unloads the extension: hang or crash 
// (e.g. issues 292 & 380) on Windows 7 while saving ini files (those lists were 
// already unallocated..)
// slots lists are allocated on the heap for the same reason..
// anyway, no prob here because application exit will destroy the entire runtime 
// context regardless.

WDL_PtrList<WDL_FastString> g_autoSaveDirs;
WDL_PtrList<WDL_FastString> g_autoFillDirs;
WDL_PtrList<PathSlotItem> g_dragPathSlotItems; 
WDL_PtrList<FileSlotList> g_slots;

// shared between the list view & the wnd, other prefs are member variables of SNM_ResourceWnd
int g_type = -1;

WDL_FastString g_filter(FILTER_DEFAULT_STR);
int g_filterPref = 1; // bitmask: &1 = filter by name, &2 = filter by path, &4 = filter by comment

int g_prjLoaderStartPref = -1; // 1-based
int g_prjLoaderEndPref = -1; // 1-based

int g_dblClickType[SNM_MAX_SLOT_TYPES];
int g_dblClickTo = 0; // for fx chains only


// helper funcs
FileSlotList* GetCurList() {
	return g_slots.Get(g_type);
}
WDL_FastString* GetCurAutoSaveDir() {
	return g_autoSaveDirs.Get(g_type);
}
WDL_FastString* GetCurAutoFillDir() {
	return g_autoFillDirs.Get(g_type);
}
bool IsFiltered() {
	return (g_filter.GetLength() && strcmp(g_filter.Get(), FILTER_DEFAULT_STR));
}


///////////////////////////////////////////////////////////////////////////////
// FileSlotList
///////////////////////////////////////////////////////////////////////////////

void FileSlotList::ClearSlot(int _slot, bool _guiUpdate) {
	if (_slot >=0 && _slot < GetSize())	{
		Get(_slot)->Clear();
		if (_guiUpdate && g_pResourcesWnd)
			g_pResourcesWnd->Update();
	}
}

void FileSlotList::ClearSlotPrompt(COMMAND_T* _ct) {
	int slot = PromptForInteger(SNM_CMD_SHORTNAME(_ct), "Slot", 1, GetSize()); //loops on err
	if (slot == -1) return; // user has cancelled
	else ClearSlot(slot);
}

// returns false if cancelled
bool FileSlotList::BrowseSlot(int _slot, char* _fn, int _fnSz)
{
	bool ok = false;
	if (_slot >= 0 && _slot < GetSize())
	{
		char title[128]="", filename[BUFFER_SIZE]="", fileFilter[512]="";
		_snprintf(title, 128, "S&M - Load %s (slot %d)", m_desc.Get(), _slot+1);
		GetFileFilter(fileFilter, 512);
		if (BrowseResourcePath(title, m_resDir.Get(), fileFilter, filename, BUFFER_SIZE, true))
		{
			if (_fn)
				lstrcpyn(_fn, filename, _fnSz);

			if (SetFromFullPath(_slot, filename))
			{
				if (g_pResourcesWnd)
					g_pResourcesWnd->Update();
				ok = true;
			}
		}
	}
	return ok;
}

bool FileSlotList::GetOrBrowseSlot(int _slot, char* _fn, int _fnSz, bool _errMsg)
{
	bool ok = false;
	if (_slot >= 0 && _slot < GetSize())
	{
		if (Get(_slot)->IsDefault())
			ok = BrowseSlot(_slot, _fn, _fnSz);
		else if (GetFullPath(_slot, _fn, _fnSz))
			ok = FileExistsErrMsg(_fn, _errMsg);
	}
	return ok;
}

// returns NULL if failed, otherwise it's up to the caller to free the returned string
WDL_FastString* FileSlotList::GetOrPromptOrBrowseSlot(const char* _title, int _slot)
{
	WDL_FastString* fnStr = NULL;
	if (_slot < 0 || _slot < GetSize())
	{
		// prompt for slot if needed
		if (_slot == -1) _slot = PromptForInteger(_title, "Slot", 1, GetSize()); // loops on err
		if (_slot == -1) return NULL; // user has cancelled

		char fn[BUFFER_SIZE]="";
		if (GetOrBrowseSlot(_slot, fn, BUFFER_SIZE, _slot < 0 || !Get(_slot)->IsDefault()))
			fnStr = new WDL_FastString(fn);
	}
	return fnStr;
}

void FileSlotList::EditSlot(int _slot)
{
	if (_slot >= 0 && _slot < GetSize())
	{
		char fullPath[BUFFER_SIZE] = "";
		if (GetFullPath(_slot, fullPath, BUFFER_SIZE) && FileExistsErrMsg(fullPath, true))
		{
#ifdef _WIN32
			WinSpawnNotepad(fullPath);
#else
			WDL_FastString chain;
			if (LoadChunk(fullPath, &chain, false))
			{
				char title[64] = "";
				_snprintf(title, 64, "S&M - %s (slot %d)", m_desc.Get(), _slot+1);
				SNM_ShowMsg(chain.Get(), title);
			}
#endif
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// SNM_ResourceView
///////////////////////////////////////////////////////////////////////////////

static SWS_LVColumn g_fxChainListCols[] = { {65,2,"Slot"}, {100,1,"Name"}, {250,2,"Path"}, {200,1,"Comment"} };

SNM_ResourceView::SNM_ResourceView(HWND hwndList, HWND hwndEdit)
:SWS_ListView(hwndList, hwndEdit, 4, g_fxChainListCols, "Resources View State", false)
{
//	ListView_SetExtendedListViewStyleEx(hwndList, ListView_GetExtendedListViewStyle(hwndList), LVS_EX_GRIDLINES);
}

void SNM_ResourceView::GetItemText(SWS_ListItem* item, int iCol, char* str, int iStrMax)
{
	if (str) *str = '\0';
	if (PathSlotItem* pItem = (PathSlotItem*)item)
	{
		switch (iCol)
		{
			case 0: {
				int slot = GetCurList()->Find(pItem);
				if (slot >= 0)
				{
					slot++;
					if (g_type == SNM_SLOT_PRJ && isProjectLoaderConfValid())
						_snprintf(str, iStrMax, "%5.d %s", slot, slot<g_prjLoaderStartPref || slot>g_prjLoaderEndPref ? "  " : 
							g_prjLoaderStartPref==slot ? "->" : g_prjLoaderEndPref==slot ? "<-" :  "--");
					else
						_snprintf(str, iStrMax, "%5.d", slot);
				}
				break;
			}
			case 1:
				GetFilenameNoExt(pItem->m_shortPath.Get(), str, iStrMax);
				break;
			case 2:
				lstrcpyn(str, pItem->m_shortPath.Get(), iStrMax);
				break;
			case 3:
				lstrcpyn(str, pItem->m_comment.Get(), iStrMax);
				break;
		}
	}
}

bool SNM_ResourceView::IsEditListItemAllowed(SWS_ListItem* item, int iCol)
{
	if (PathSlotItem* pItem = (PathSlotItem*)item)
		if (!pItem->IsDefault())
		{
			int slot = GetCurList()->Find(pItem);
			switch (iCol)
			{		
				// file renaming
				case 1: {
					char fn[BUFFER_SIZE] = "";
					return (GetCurList()->GetFullPath(slot, fn, BUFFER_SIZE) && FileExistsErrMsg(fn, false));
				}
				// comment
				case 3:
					return true;
			}
		}
	return false;
}

void SNM_ResourceView::SetItemText(SWS_ListItem* item, int iCol, const char* str)
{
	PathSlotItem* pItem = (PathSlotItem*)item;
	int slot = GetCurList()->Find(pItem);
	if (pItem && slot >=0)
	{
		switch (iCol)
		{
			// file renaming
			case 1:
			{
				char fn[BUFFER_SIZE] = "";
				if (GetCurList()->GetFullPath(slot, fn, BUFFER_SIZE) && !pItem->IsDefault() && FileExistsErrMsg(fn, true))
				{
					const char* ext = GetFileExtension(fn);
					char newFn[BUFFER_SIZE]="";
					lstrcpyn(newFn, fn, BUFFER_SIZE);
					if (char* p = strrchr(newFn, PATH_SLASH_CHAR)) *p = '\0';
					else break; // safety

					_snprintf(newFn, BUFFER_SIZE, "%s%c%s.%s", newFn, PATH_SLASH_CHAR, str, ext);
					if (FileExists(newFn)) 
					{
						char buf[BUFFER_SIZE];
						_snprintf(buf, BUFFER_SIZE, "File already exists. Overwrite ?\n%s", newFn);
						int res = MessageBox(g_hwndParent, buf, "S&M - Warning", MB_YESNO);
						if (res == IDYES) {
							if (!SNM_DeleteFile(newFn, false))
								break;
						}
						else 
							break;
					}
					if (MoveFile(fn, newFn) && GetCurList()->SetFromFullPath(slot, newFn))
						ListView_SetItemText(m_hwndList, GetEditingItem(), DisplayToDataCol(2), (LPSTR)pItem->m_shortPath.Get());
						// ^^ direct GUI update 'cause Update() is no-op when editing
				}
			}
			break;
			// comment
			case 3:
				pItem->m_comment.Set(str);
				pItem->m_comment.Ellipsize(128,128);
				Update();
				break;
		}
	}
}

void SNM_ResourceView::OnItemDblClk(SWS_ListItem* item, int iCol)
{
	PathSlotItem* pItem = (PathSlotItem*)item;
	int slot = GetCurList()->Find(pItem);
	if (pItem && slot >= 0) 
	{
		bool wasDefaultSlot = pItem->IsDefault();
		switch(g_type)
		{
			case SNM_SLOT_FXC:
				switch(g_dblClickTo) {
					case 0:
						applyTracksFXChainSlot(!g_dblClickType[g_type]?FXC_APPLY_TR_STR:FXC_PASTE_TR_STR, slot, !g_dblClickType[g_type], false);
						break;
					case 1:
						applyTracksFXChainSlot(!g_dblClickType[g_type]?FXCIN_APPLY_TR_STR:FXCIN_PASTE_TR_STR, slot, !g_dblClickType[g_type], g_bv4);
						break;
					case 2:
						applyTakesFXChainSlot(!g_dblClickType[g_type]?FXC_APPLY_TAKE_STR:FXC_PASTE_TAKE_STR, slot, true, !g_dblClickType[g_type]);
						break;
					case 3:
						applyTakesFXChainSlot(!g_dblClickType[g_type]?FXC_APPLY_ALLTAKES_STR:FXC_PASTE_ALLTAKES_STR, slot, false, !g_dblClickType[g_type]);
						break;
				}
				break;
			case SNM_SLOT_TR:
				switch(g_dblClickType[g_type]) {
					case 0:
						applyOrImportTrackSlot(TRT_APPLY_STR, slot, false, false, false);
						break;
					case 1:
						applyOrImportTrackSlot(TRT_APPLY_WITEMS_STR, slot, false, true, false);
						break;
					case 2:
						applyOrImportTrackSlot(TRT_IMPORT_STR, slot, true, false, false);
						break;
					case 3:
						replaceOrPasteItemsFromTrackSlot(TRT_PASTE_ITEMS_STR, slot, true);
						break;
					case 4:
						replaceOrPasteItemsFromTrackSlot(TRT_APPLY_ITEMS_STR, slot, false);
						break;
				}
				break;
			case SNM_SLOT_PRJ:
				switch(g_dblClickType[g_type]) {
					case 0:
						loadOrSelectProjectSlot(PRJ_SELECT_LOAD_STR, slot, false);
						break;
					case 1:
						loadOrSelectProjectSlot(PRJ_SELECT_LOAD_TAB_STR, slot, true);
						break;
				}
				break;
			case SNM_SLOT_MEDIA:
			{
				int insertMode = -1;
				switch(g_dblClickType[g_type]) 
				{
					case 0: TogglePlaySelTrackMediaSlot(MED_PLAY_STR, slot, false); break;
					case 1: TogglePlaySelTrackMediaSlot(MED_LOOP_STR, slot, true); break;
					case 2: insertMode = 0; break;
					case 3: insertMode = 1; break;
					case 4: insertMode = 3; break;
				}
				if (insertMode >= 0)
					InsertMediaSlot(MED_ADD_STR, slot, insertMode);
				break;
			}
#ifdef _WIN32
	 		case SNM_SLOT_THM:
				LoadThemeSlot(THM_LOAD_STR, slot);
				break;
#endif
		}

		// update if the slot changed
		if (wasDefaultSlot && !pItem->IsDefault())
			Update();
	}
}

void SNM_ResourceView::GetItemList(SWS_ListItemList* pList)
{
	if (IsFiltered())
	{
		char buf[BUFFER_SIZE] = "";
		LineParser lp(false);
		if (!lp.parse(g_filter.Get()))
		{
			for (int i = 0; i < GetCurList()->GetSize(); i++)
			{
				if (PathSlotItem* item = GetCurList()->Get(i))
				{
					bool match = false;
					for (int j=0; !match && j < lp.getnumtokens(); j++)
					{
						if (g_filterPref&1) // name
						{
							GetFilenameNoExt(item->m_shortPath.Get(), buf, BUFFER_SIZE);
							match |= (stristr(buf, lp.gettoken_str(j)) != NULL);
						}
						if (!match && (g_filterPref&2)) // path
						{
							if (GetCurList()->GetFullPath(i, buf, BUFFER_SIZE))
								if (char* p = strrchr(buf, PATH_SLASH_CHAR)) {
									*p = '\0';
									match |= (stristr(buf, lp.gettoken_str(j)) != NULL);
								}
						}
						if (!match && (g_filterPref&4)) // comment
							match |= (stristr(item->m_comment.Get(), lp.gettoken_str(j)) != NULL);
					}
					if (match)
						pList->Add((SWS_ListItem*)item);
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < GetCurList()->GetSize(); i++)
			pList->Add((SWS_ListItem*)GetCurList()->Get(i));
	}
}

void SNM_ResourceView::OnBeginDrag(SWS_ListItem* _item)
{
#ifdef _WIN32
	g_dragPathSlotItems.Empty(false);

	// Store dragged items (for internal d'n'd) + full paths + get the amount of memory needed
	int iMemNeeded = 0, x=0;
	WDL_PtrList_DeleteOnDestroy<WDL_FastString> fullPaths;
	PathSlotItem* pItem = (PathSlotItem*)EnumSelected(&x);
	while(pItem) {
		int slot = GetCurList()->Find(pItem);
		char fullPath[BUFFER_SIZE] = "";
		if (GetCurList()->GetFullPath(slot, fullPath, BUFFER_SIZE))
		{
			bool empty = (pItem->IsDefault() || *fullPath == '\0');
			iMemNeeded += (int)((empty ? strlen(DRAGDROP_EMPTY_SLOT) : strlen(fullPath)) + 1);
			fullPaths.Add(new WDL_FastString(empty ? DRAGDROP_EMPTY_SLOT : fullPath));
			g_dragPathSlotItems.Add(pItem);
		}
		pItem = (PathSlotItem*)EnumSelected(&x);
	}
	if (!iMemNeeded)
		return;

	iMemNeeded += sizeof(DROPFILES) + 1;

	HGLOBAL hgDrop = GlobalAlloc (GHND | GMEM_SHARE, iMemNeeded);
	DROPFILES* pDrop = NULL;
	if (hgDrop)
		pDrop = (DROPFILES*)GlobalLock(hgDrop); // 'spose should do some error checking...
	
	// for safety..
	if (!hgDrop || !pDrop)
		return;

	pDrop->pFiles = sizeof(DROPFILES);
	pDrop->fWide = false;
	char* pBuf = (char*)pDrop + pDrop->pFiles;

	// Add the files to the DROPFILES struct, double-NULL terminated
	for (int i=0; i < fullPaths.GetSize(); i++) {
		strcpy(pBuf, fullPaths.Get(i)->Get());
		pBuf += strlen(pBuf) + 1;
	}
	*pBuf = 0;

	GlobalUnlock(hgDrop);
	FORMATETC etc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };
	stgmed.hGlobal = hgDrop;

	SWS_IDataObject* dataObj = new SWS_IDataObject(&etc, &stgmed);
	SWS_IDropSource* dropSrc = new SWS_IDropSource;
	DWORD effect;

	DoDragDrop(dataObj, dropSrc, DROPEFFECT_COPY, &effect);
#endif
}


///////////////////////////////////////////////////////////////////////////////
// SNM_ResourceWnd
///////////////////////////////////////////////////////////////////////////////

SNM_ResourceWnd::SNM_ResourceWnd()
	: SWS_DockWnd(IDD_SNM_FXCHAINLIST, "Resources", "SnMResources", 30006, SWSGetCommandID(OpenResourceView))
{
	m_previousType = g_type;
	m_autoSaveTrTmpltWithItemsPref = true;

	// Must call SWS_DockWnd::Init() to restore parameters and open the window if necessary
	Init();
}

INT_PTR SNM_ResourceWnd::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int sListOldColors[LISTVIEW_COLORHOOK_STATESIZE];
	if (ListView_HookThemeColorsMessage(m_hwnd, uMsg, lParam, sListOldColors, IDC_LIST, 0, 0))
		return 1;
	return SWS_DockWnd::WndProc(uMsg, wParam, lParam);
}

void SNM_ResourceWnd::SetType(int _type)
{
	m_previousType = g_type;
	g_type = _type;
	m_cbType.SetCurSel(_type);
	if (m_previousType != g_type)
	{
		FillDblClickTypeCombo();
		Update();
	}
}

void SNM_ResourceWnd::Update()
{
	if (m_pLists.GetSize())
		m_pLists.Get(0)->Update();
	m_parentVwnd.RequestRedraw(NULL);
}

//JFB!!! hard coded labels..
void SNM_ResourceWnd::FillDblClickTypeCombo()
{
	m_cbDblClickType.Empty();
	switch(g_type)
	{
		case SNM_SLOT_FXC:
			m_cbDblClickType.AddItem("Paste (replace)");
			m_cbDblClickType.AddItem("Paste");
			break;
		case SNM_SLOT_TR:
			m_cbDblClickType.AddItem("Apply to sel tracks (w/o items)");
			m_cbDblClickType.AddItem("Apply to sel tracks (w/ items)");
			m_cbDblClickType.AddItem("Import tracks");
			m_cbDblClickType.AddItem("Paste items to sel tracks");
			m_cbDblClickType.AddItem("Replace items of sel tracks");
			break;
		case SNM_SLOT_PRJ:
			m_cbDblClickType.AddItem("Load/select project");
			m_cbDblClickType.AddItem("Load/select project tab");
			break;
		case SNM_SLOT_MEDIA:
			m_cbDblClickType.AddItem("Play in sel tracks (toggle)");
			m_cbDblClickType.AddItem("Play/loop in sel tracks (toggle)");
			m_cbDblClickType.AddItem("Add to current track");
			m_cbDblClickType.AddItem("Add to new track");
			m_cbDblClickType.AddItem("Add to sel items as takes");
			break;
#ifdef _WIN32
		case SNM_SLOT_THM:
			m_cbDblClickType.AddItem("Load theme");
			break;
#endif
	}
	m_cbDblClickType.SetCurSel(g_dblClickType[g_type]);
}

void SNM_ResourceWnd::OnInitDlg()
{
	m_resize.init_item(IDC_LIST, 0.0, 0.0, 1.0, 1.0);
	m_resize.init_item(IDC_FILTER, 1.0, 0.0, 1.0, 0.0);
	SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_FILTER), GWLP_USERDATA, 0xdeadf00b);
/*JFB commented: seems useless since r488 theming updates
#ifndef _WIN32
	// Realign the filter box on OSX
	HWND hFilter = GetDlgItem(m_hwnd, IDC_FILTER);
	RECT rFilter;
	GetWindowRect(hFilter, &rFilter);
	ScreenToClient(m_hwnd,(LPPOINT)&rFilter);
	ScreenToClient(m_hwnd,((LPPOINT)&rFilter)+1);
	SetWindowPos(hFilter, NULL, rFilter.left - 25, rFilter.top - 1, abs(rFilter.right - rFilter.left) - 10, abs(rFilter.bottom - rFilter.top), SWP_NOACTIVATE | SWP_NOZORDER);
#endif
*/
	m_pLists.Add(new SNM_ResourceView(GetDlgItem(m_hwnd, IDC_LIST), GetDlgItem(m_hwnd, IDC_EDIT)));
	SNM_ThemeListView(m_pLists.Get(0));


	// load prefs 

	// BOUNDED: safety, some custom slot types may have been removed..
	g_type = BOUNDED((int)GetPrivateProfileInt("RESOURCE_VIEW", "Type", SNM_SLOT_FXC, g_SNMIniFn.Get()), 0, g_slots.GetSize()-1);

	g_filterPref = GetPrivateProfileInt("RESOURCE_VIEW", "FilterByPath", 1, g_SNMIniFn.Get());
	g_dblClickTo = GetPrivateProfileInt("RESOURCE_VIEW", "DblClick_To", 0, g_SNMIniFn.Get());
	m_autoSaveFXChainPref = GetPrivateProfileInt("RESOURCE_VIEW", "AutoSaveFXChain", FXC_AUTOSAVE_PREF_TRACK, g_SNMIniFn.Get());
	m_autoSaveTrTmpltWithItemsPref = (GetPrivateProfileInt("RESOURCE_VIEW", "AutoSaveTrTemplateWithItems", 1, g_SNMIniFn.Get()) == 1);
	g_prjLoaderStartPref = GetPrivateProfileInt("RESOURCE_VIEW", "ProjectLoaderStartSlot", 1, g_SNMIniFn.Get());
	g_prjLoaderEndPref = GetPrivateProfileInt("RESOURCE_VIEW", "ProjectLoaderEndSlot", g_slots.Get(SNM_SLOT_PRJ)->GetSize(), g_SNMIniFn.Get());

	// auto-save, auto-fill directories, etc..
	g_autoSaveDirs.Empty(true);
	g_autoFillDirs.Empty(true);
	char defaultPath[BUFFER_SIZE]="", path[BUFFER_SIZE]="", iniKey[64]="";
	for (int i=0; i < g_slots.GetSize(); i++)
	{
		_snprintf(defaultPath, BUFFER_SIZE, "%s%c%s", GetResourcePath(), PATH_SLASH_CHAR, g_slots.Get(i)->GetResourceDir());

		_snprintf(iniKey, 64, "AutoSaveDir%s", g_slots.Get(i)->GetResourceDir(false));
		GetPrivateProfileString("RESOURCE_VIEW", iniKey, defaultPath, path, BUFFER_SIZE, g_SNMIniFn.Get());
		g_autoSaveDirs.Add(new WDL_FastString(path));
		_snprintf(iniKey, 64, "AutoFillDir%s", g_slots.Get(i)->GetResourceDir(false));
		GetPrivateProfileString("RESOURCE_VIEW", iniKey, defaultPath, path, BUFFER_SIZE, g_SNMIniFn.Get());
		g_autoFillDirs.Add(new WDL_FastString(path));

		_snprintf(iniKey, 64, "DblClick%s", g_slots.Get(i)->GetResourceDir(false));
		g_dblClickType[i] = GetPrivateProfileInt("RESOURCE_VIEW", iniKey, 0, g_SNMIniFn.Get());
	}

	// WDL GUI init
	m_vwnd_painter.SetGSC(WDL_STYLE_GetSysColor);
	m_parentVwnd.SetRealParent(m_hwnd);

	m_cbType.SetID(COMBOID_TYPE);
	for (int i=0; i < g_slots.GetSize(); i++)
		m_cbType.AddItem(g_slots.Get(i)->GetMenuDesc());
	m_cbType.SetCurSel(g_type);
	m_parentVwnd.AddChild(&m_cbType);

	m_cbDblClickType.SetID(COMBOID_DBLCLICK_TYPE);
	FillDblClickTypeCombo(); //JFB single call to SetType() instead !?
	m_parentVwnd.AddChild(&m_cbDblClickType);

	m_cbDblClickTo.SetID(COMBOID_DBLCLICK_TO);
	m_cbDblClickTo.AddItem("Tracks");
	m_cbDblClickTo.AddItem("Tracks (input FX)");
	m_cbDblClickTo.AddItem("Items");
	m_cbDblClickTo.AddItem("Items (all takes)");
	m_cbDblClickTo.SetCurSel(g_dblClickTo);
	m_parentVwnd.AddChild(&m_cbDblClickTo);

	m_btnAutoSave.SetID(BUTTONID_AUTO_SAVE);
	m_parentVwnd.AddChild(&m_btnAutoSave);

	m_txtDblClickType.SetID(TXTID_DBL_TYPE);
	m_txtDblClickType.SetText("Dbl-click to:");
	m_parentVwnd.AddChild(&m_txtDblClickType);

	m_txtDblClickTo.SetID(TXTID_DBL_TO);
	m_txtDblClickTo.SetText("To selected:");
	m_parentVwnd.AddChild(&m_txtDblClickTo);

	// restores the text filter when docking/undocking + indirect call to Update() !
	SetDlgItemText(GetHWND(), IDC_FILTER, g_filter.Get());

/* Perfs: see above comment
	Update();
*/
}

void SNM_ResourceWnd::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int x=0;
	PathSlotItem* item = (PathSlotItem*)m_pLists.Get(0)->EnumSelected(&x);
	int slot = GetCurList()->Find(item);
	bool wasDefaultSlot = item ? item->IsDefault() : false;

	switch(LOWORD(wParam))
	{
		case IDC_FILTER:
			if (HIWORD(wParam)==EN_CHANGE)
			{
				char cFilter[128];
				GetWindowText(GetDlgItem(m_hwnd, IDC_FILTER), cFilter, 128);
				g_filter.Set(cFilter);
				Update();
			}
#ifdef _WIN32
			else if (HIWORD(wParam)==EN_SETFOCUS)
			{
				HWND hFilt = GetDlgItem(m_hwnd, IDC_FILTER);
				char cFilter[128];
				GetWindowText(hFilt, cFilter, 128);
				if (!strcmp(cFilter, FILTER_DEFAULT_STR))
					SetWindowText(hFilt, "");
			}
			else if (HIWORD(wParam)==EN_KILLFOCUS)
			{
				HWND hFilt = GetDlgItem(m_hwnd, IDC_FILTER);
				char cFilter[128];
				GetWindowText(hFilt, cFilter, 128);
				if (*cFilter == '\0') 
					SetWindowText(hFilt, FILTER_DEFAULT_STR);
			}
#endif
			break;

		// ***** Common *****
		case ADD_SLOT_MSG:
			AddSlot(true);
			break;
		case INSERT_SLOT_MSG:
			InsertAtSelectedSlot(true);
			break;
		case CLEAR_SLOTS_MSG:
			ClearDeleteSelectedSlots(0, true);
			break;
		case DEL_SLOTS_MSG:
			ClearDeleteSelectedSlots(1, true);
			break;
		case DEL_FILES_MSG:
			ClearDeleteSelectedSlots(6, true);
			break;
		case LOAD_MSG:
			if (item) GetCurList()->BrowseSlot(slot);
			break;
		case EDIT_MSG:
			if (item) GetCurList()->EditSlot(slot);
			break;
		case EXPLORE_MSG:
		{
			char fullPath[BUFFER_SIZE] = "";
			if (GetCurList()->GetFullPath(slot, fullPath, BUFFER_SIZE))
				if (char* p = strrchr(fullPath, PATH_SLASH_CHAR)) {
					*(p+1) = '\0'; // ShellExecute() is KO otherwie..
					ShellExecute(NULL, "open", fullPath, NULL, NULL, SW_SHOWNORMAL);
				}
			break;
		}
		case AUTO_FILL_DIR_MSG:
		{
			char startPath[BUFFER_SIZE] = "";
			if (BrowseForDirectory("Auto-fill directory", GetCurAutoFillDir()->Get(), startPath, BUFFER_SIZE)) {
				GetCurAutoFillDir()->Set(startPath);
				AutoFill(startPath);
			}
			break;
		}
		case AUTO_FILL_PRJ_MSG:
		{
			char startPath[BUFFER_SIZE] = "";
			GetProjectPath(startPath, BUFFER_SIZE);
			AutoFill(startPath); // will look for sub-directories
			break;
		}
		case AUTO_FILL_DEFAULT_MSG:
		{
			char startPath[BUFFER_SIZE] = "";
			_snprintf(startPath, BUFFER_SIZE, "%s%c%s", GetResourcePath(), PATH_SLASH_CHAR, GetCurList()->GetResourceDir());
			AutoFill(startPath);
			break;
		}
		case BUTTONID_AUTO_SAVE:
		case AUTOSAVE_MSG:
			AutoSave();
			break;
		case AUTOSAVE_DIR_MSG:
		{
			char path[BUFFER_SIZE] = "";
			if (BrowseForDirectory("Set auto-save directory", GetCurAutoSaveDir()->Get(), path, BUFFER_SIZE))
				GetCurAutoSaveDir()->Set(path);
			break;
		}
		case AUTOSAVE_DIR_PRJ_MSG:
		{
			char prjPath[BUFFER_SIZE] = "", path[BUFFER_SIZE] = "";
			GetProjectPath(prjPath, BUFFER_SIZE);
			if (!strstr(prjPath, GetResourcePath()))
				_snprintf(path, BUFFER_SIZE, "%s%c%s", prjPath, PATH_SLASH_CHAR, GetCurList()->GetResourceDir());
			else
				_snprintf(path, BUFFER_SIZE, "%s%c%s", GetResourcePath(), PATH_SLASH_CHAR, GetCurList()->GetResourceDir());
			if (!FileExists(path))
				CreateDirectory(path, NULL);
			GetCurAutoSaveDir()->Set(path);
			break;
		}
		case AUTOSAVE_DIR_DEFAULT_MSG:
		{
			char path[BUFFER_SIZE] = "";
			_snprintf(path, BUFFER_SIZE, "%s%c%s", GetResourcePath(), PATH_SLASH_CHAR, GetCurList()->GetResourceDir());
			if (!FileExists(path))
				CreateDirectory(path, NULL);
			GetCurAutoSaveDir()->Set(path);
			break;
		}
		case FILTER_BY_NAME_MSG:
			if (g_filterPref&1) g_filterPref &= 6; // 110
			else g_filterPref |= 1;
			Update();
//			SetFocus(GetDlgItem(m_hwnd, IDC_FILTER));
			break;
		case FILTER_BY_PATH_MSG:
			if (g_filterPref&2) g_filterPref &= 5; // 101
			else g_filterPref |= 2;
			Update();
//			SetFocus(GetDlgItem(m_hwnd, IDC_FILTER));
			break;
		case FILTER_BY_COMMENT_MSG:
			if (g_filterPref&4) g_filterPref &= 3; // 011
			else g_filterPref |= 4;
			Update();
//			SetFocus(GetDlgItem(m_hwnd, IDC_FILTER));
			break;
		case RENAME_MSG:
			if (item) {
				char fullPath[BUFFER_SIZE] = "";
				if (GetCurList()->GetFullPath(slot, fullPath, BUFFER_SIZE) && FileExistsErrMsg(fullPath, true))
					m_pLists.Get(0)->EditListItem((SWS_ListItem*)item, 1);
			}
			break;

		// ***** FX chain *****
		case FXC_COPY_MSG:
			if (item) copyFXChainSlotToClipBoard(slot);
			break;
		case FXC_APPLY_TR_MSG:
		case FXC_PASTE_TR_MSG:
			if (item && slot >= 0) {
				applyTracksFXChainSlot(LOWORD(wParam)==FXC_APPLY_TR_MSG?FXC_APPLY_TR_STR:FXC_PASTE_TR_STR, slot, LOWORD(wParam)==FXC_APPLY_TR_MSG, false);
				if (wasDefaultSlot && !item->IsDefault()) // slot has been filled ?
					Update();
			}
			break;
		case FXC_APPLY_TAKE_MSG:
		case FXC_PASTE_TAKE_MSG:
			if (item && slot >= 0) {
				applyTakesFXChainSlot(LOWORD(wParam)==FXC_APPLY_TAKE_MSG?FXC_APPLY_TAKE_STR:FXC_PASTE_TAKE_STR, slot, true, LOWORD(wParam)==FXC_APPLY_TAKE_MSG);
				if (wasDefaultSlot && !item->IsDefault()) // slot has been filled ?
					Update();
			}
			break;
		case FXC_APPLY_ALLTAKES_MSG:
		case FXC_PASTE_ALLTAKES_MSG:
			if (item && slot >= 0) {
				applyTakesFXChainSlot(LOWORD(wParam)==FXC_APPLY_ALLTAKES_MSG?FXC_APPLY_ALLTAKES_STR:FXC_PASTE_ALLTAKES_STR, slot, false, LOWORD(wParam)==FXC_APPLY_ALLTAKES_MSG);
				if (wasDefaultSlot && !item->IsDefault()) // slot has been filled ?
					Update();
			}
			break;
		case FXC_AUTOSAVE_INPUT_FX:
			m_autoSaveFXChainPref = FXC_AUTOSAVE_PREF_INPUT_FX;
			break;
		case FXC_AUTOSAVE_TR_MSG:
			m_autoSaveFXChainPref = FXC_AUTOSAVE_PREF_TRACK;
			break;
		case FXC_AUTOSAVE_ITEM_MSG:
			m_autoSaveFXChainPref = FXC_AUTOSAVE_PREF_ITEM;
			break;

		// ***** Track template *****
		case TRT_APPLY_MSG:
		case TRT_APPLY_WITEMS_MSG:
			if (item && slot >= 0) {
				applyOrImportTrackSlot(LOWORD(wParam)==TRT_APPLY_MSG?TRT_APPLY_STR:TRT_APPLY_WITEMS_STR, slot, false, LOWORD(wParam)==TRT_APPLY_WITEMS_MSG, false);
				if (wasDefaultSlot && !item->IsDefault()) // slot has been filled ?
					Update();
			}
			break;
		case TRT_IMPORT_MSG:
			if (item && slot >= 0) {
				applyOrImportTrackSlot(TRT_IMPORT_STR, slot, true, false, false);
				if (wasDefaultSlot && !item->IsDefault()) // slot has been filled ?
					Update();
			}
			break;
		case TRT_REPLACE_ITEMS_MSG:
		case TRT_PASTE_ITEMS_MSG:
			if (item && slot >= 0) {
				replaceOrPasteItemsFromTrackSlot(LOWORD(wParam)==TRT_REPLACE_ITEMS_MSG?TRT_APPLY_ITEMS_STR:TRT_PASTE_ITEMS_STR, slot, LOWORD(wParam)==TRT_PASTE_ITEMS_MSG);
				if (wasDefaultSlot && !item->IsDefault()) // slot has been filled ?
					Update();
			}
			break;
		case TRT_AUTOSAVE_WITEMS_MSG:
			m_autoSaveTrTmpltWithItemsPref = !m_autoSaveTrTmpltWithItemsPref;
			break;

		// ***** Project template *****
		case PRJ_SELECT_LOAD_MSG:
			if (item && slot >= 0) {
				loadOrSelectProjectSlot(PRJ_SELECT_LOAD_STR, slot, false);
				if (wasDefaultSlot && !item->IsDefault()) // slot has been filled ?
					Update();
			}
			break;
		case PRJ_SELECT_LOAD_TAB_MSG:
		{
			bool updt = false;
			while(item) {
				slot = GetCurList()->Find(item);
				wasDefaultSlot = item->IsDefault();
				loadOrSelectProjectSlot(PRJ_SELECT_LOAD_TAB_STR, slot, true);
				updt |= (wasDefaultSlot && !item->IsDefault()); // slot has been filled ?
				item = (PathSlotItem*)m_pLists.Get(0)->EnumSelected(&x);
			}
			if (updt) Update();
			break;
		}
		case PRJ_AUTOFILL_RECENTS_MSG:
		{
			int startSlot = GetCurList()->GetSize();
			int nbRecents = GetPrivateProfileInt("REAPER", "numrecent", 0, get_ini_file());
			nbRecents = min(98, nbRecents); // just in case: 2 digits max..
			if (nbRecents)
			{
				char key[16], path[BUFFER_SIZE];
				WDL_PtrList_DeleteOnDestroy<WDL_FastString> prjs;
				for (int i=0; i < nbRecents; i++) {
					_snprintf(key, 9, "recent%02d", i+1);
					GetPrivateProfileString("Recent", key, "", path, BUFFER_SIZE, get_ini_file());
					if (*path)
						prjs.Add(new WDL_FastString(path));
				}
				for (int i=0; i < prjs.GetSize(); i++)
					if (GetCurList()->FindByResFulltPath(prjs.Get(i)->Get()) < 0) // skip if already present
						GetCurList()->AddSlot(prjs.Get(i)->Get());
			}
			if (startSlot != GetCurList()->GetSize()) {
				Update();
				SelectBySlot(startSlot, GetCurList()->GetSize());
			}
			else
				MessageBox(GetMainHwnd(), "No valid recent projects found!", "S&M - Warning", MB_OK);
			break;
		}
		case PRJ_LOADER_CONF_MSG:
			projectLoaderConf(NULL);
			break;
		case PRJ_LOADER_SET_MSG:
		{
			int min=GetCurList()->GetSize(), max=0;
			while(item) {
				slot = GetCurList()->Find(item)+1;
				if (slot>max) max = slot;
				if (slot<min) min = slot;
				item = (PathSlotItem*)m_pLists.Get(0)->EnumSelected(&x);
			}
			if (max>min) {
				g_prjLoaderStartPref = min;
				g_prjLoaderEndPref = max;
				Update();
			}
			break;
		}
		case PRJ_LOADER_CLEAR_MSG:
			g_prjLoaderStartPref = g_prjLoaderEndPref = -1;
			Update();
			break;

		// ***** media files *****
		case MED_PLAY_MSG:
		case MED_LOOP_MSG:
			while(item) {
				slot = GetCurList()->Find(item);
				wasDefaultSlot = item->IsDefault();
				TogglePlaySelTrackMediaSlot(MED_PLAYLOOP_STR, slot, LOWORD(wParam)==MED_LOOP_MSG);
				item = (PathSlotItem*)m_pLists.Get(0)->EnumSelected(&x);
			}
			break;
		case MED_ADD_CURTR_MSG:
		case MED_ADD_NEWTR_MSG:
		case MED_ADD_TAKES_MSG:
			while(item) {
				slot = GetCurList()->Find(item);
				wasDefaultSlot = item->IsDefault();
				InsertMediaSlot(MED_ADD_STR, slot, LOWORD(wParam)==MED_ADD_CURTR_MSG ? 0 : LOWORD(wParam)==MED_ADD_NEWTR_MSG ? 1 : 3);
				item = (PathSlotItem*)m_pLists.Get(0)->EnumSelected(&x);
			}
			break;

		// ***** theme *****
#ifdef _WIN32
		case THM_LOAD_MSG:
			LoadThemeSlot(THM_LOAD_STR, slot);
			break;
#endif

		// ***** WDL GUI & others *****
		case COMBOID_TYPE:
			if (HIWORD(wParam)==CBN_SELCHANGE)
			{
				// stop cell editing (changing the list content would be ignored otherwise
				// => dropdown box & list box unsynchronized)
				m_pLists.Get(0)->EditListItemEnd(false);

				SetType(m_cbType.GetCurSel());
			}
			break;
		case COMBOID_DBLCLICK_TYPE:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				g_dblClickType[g_type] = m_cbDblClickType.GetCurSel();
			break;
		case COMBOID_DBLCLICK_TO:
			if (HIWORD(wParam)==CBN_SELCHANGE)
				g_dblClickTo = m_cbDblClickTo.GetCurSel();
			break;
		default:
			Main_OnCommand((int)wParam, (int)lParam);
			break;
	}
}

//JFB!!! some hard coded labels..
HMENU SNM_ResourceWnd::OnContextMenu(int x, int y)
{
	HMENU hMenu = CreatePopupMenu();
	int iCol;
	SWS_ListItem* item = m_pLists.Get(0)->GetHitItem(x, y, &iCol);
	PathSlotItem* pItem = (PathSlotItem*)item;
	UINT enabled = (pItem && !pItem->IsDefault()) ? MF_ENABLED : MF_GRAYED;
	if (pItem && iCol >= 0)
	{
		switch(g_type)
		{
			case SNM_SLOT_FXC:
				AddToMenu(hMenu, FXC_APPLY_TR_STR, FXC_APPLY_TR_MSG, -1, false, enabled);
				AddToMenu(hMenu, FXC_PASTE_TR_STR, FXC_PASTE_TR_MSG, -1, false, enabled);
				AddToMenu(hMenu, SWS_SEPARATOR, 0);
				AddToMenu(hMenu, FXC_APPLY_TAKE_STR, FXC_APPLY_TAKE_MSG, -1, false, enabled);
				AddToMenu(hMenu, FXC_APPLY_ALLTAKES_STR, FXC_APPLY_ALLTAKES_MSG, -1, false, enabled);
				AddToMenu(hMenu, FXC_PASTE_TAKE_STR, FXC_PASTE_TAKE_MSG, -1, false, enabled);
				AddToMenu(hMenu, FXC_PASTE_ALLTAKES_STR, FXC_PASTE_ALLTAKES_MSG, -1, false, enabled);
				AddToMenu(hMenu, SWS_SEPARATOR, 0);
				AddToMenu(hMenu, "Copy", FXC_COPY_MSG, -1, false, enabled);
				break;
			case SNM_SLOT_TR:
				AddToMenu(hMenu, TRT_APPLY_STR, TRT_APPLY_MSG, -1, false, enabled);
				AddToMenu(hMenu, TRT_APPLY_WITEMS_STR, TRT_APPLY_WITEMS_MSG, -1, false, enabled);
				AddToMenu(hMenu, TRT_IMPORT_STR, TRT_IMPORT_MSG, -1, false, enabled);
				AddToMenu(hMenu, SWS_SEPARATOR, 0);
				AddToMenu(hMenu, TRT_APPLY_ITEMS_STR, TRT_REPLACE_ITEMS_MSG, -1, false, enabled);
				AddToMenu(hMenu, TRT_PASTE_ITEMS_STR, TRT_PASTE_ITEMS_MSG, -1, false, enabled);
				break;
			case SNM_SLOT_PRJ:
			{
				AddToMenu(hMenu, PRJ_SELECT_LOAD_STR, PRJ_SELECT_LOAD_MSG, -1, false, enabled);
				AddToMenu(hMenu, PRJ_SELECT_LOAD_TAB_STR, PRJ_SELECT_LOAD_TAB_MSG, -1, false, enabled);
				AddToMenu(hMenu, SWS_SEPARATOR, 0);
				int x=0, nbsel=0; while(m_pLists.Get(0)->EnumSelected(&x))nbsel++;
				AddToMenu(hMenu, "Project loader/selecter configuration...", PRJ_LOADER_CONF_MSG);
				AddToMenu(hMenu, "Set project loader/selecter from slot selection", PRJ_LOADER_SET_MSG, -1, false, nbsel>1 ? MF_ENABLED : MF_GRAYED);
				AddToMenu(hMenu, "Clear project loader/selecter configuration", PRJ_LOADER_CLEAR_MSG, -1, false, isProjectLoaderConfValid() ? MF_ENABLED : MF_GRAYED);
				break;
			}
			case SNM_SLOT_MEDIA:
				AddToMenu(hMenu, MED_PLAY_STR, MED_PLAY_MSG, -1, false, enabled);
				AddToMenu(hMenu, MED_LOOP_STR, MED_LOOP_MSG, -1, false, enabled);
				AddToMenu(hMenu, SWS_SEPARATOR, 0);
				AddToMenu(hMenu, "Add to current track", MED_ADD_CURTR_MSG, -1, false, enabled);
				AddToMenu(hMenu, "Add to new tracks", MED_ADD_NEWTR_MSG, -1, false, enabled);
				AddToMenu(hMenu, "Add to selected items as takes", MED_ADD_TAKES_MSG, -1, false, enabled);
				break;
#ifdef _WIN32
			case SNM_SLOT_THM:
				AddToMenu(hMenu, THM_LOAD_STR, THM_LOAD_MSG, -1, false, enabled);
				break;
#endif
		}
		AddToMenu(hMenu, SWS_SEPARATOR, 0);
	}

	// always displayed, even if the list is empty
	AddToMenu(hMenu, "Add slot", ADD_SLOT_MSG, -1, false, !IsFiltered() ? MF_ENABLED : MF_GRAYED);

	if (pItem && iCol >= 0)
	{
		AddToMenu(hMenu, "Insert slot", INSERT_SLOT_MSG, -1, false, !IsFiltered() ? MF_ENABLED : MF_GRAYED);
		AddToMenu(hMenu, "Clear slots", CLEAR_SLOTS_MSG, -1, false, enabled);
		AddToMenu(hMenu, "Delete slots", DEL_SLOTS_MSG);

		AddToMenu(hMenu, SWS_SEPARATOR, 0);
		AddToMenu(hMenu, "Load slot/file...", LOAD_MSG);
		AddToMenu(hMenu, "Delete files", DEL_FILES_MSG, -1, false, enabled);
		AddToMenu(hMenu, "Rename file", RENAME_MSG, -1, false, enabled);
		if (GetCurList()->HasNotepad())
#ifdef _WIN32
			AddToMenu(hMenu, "Edit file...", EDIT_MSG, -1, false, enabled);
#else
			AddToMenu(hMenu, "Display file...", EDIT_MSG, -1, false, enabled);
#endif
		AddToMenu(hMenu, "Show path in explorer/finder...", EXPLORE_MSG, -1, false, enabled);
	}

	AddToMenu(hMenu, SWS_SEPARATOR, 0);
	HMENU hAutoFillSubMenu = CreatePopupMenu();
	AddSubMenu(hMenu, hAutoFillSubMenu, "Auto-fill");
	AddToMenu(hAutoFillSubMenu, "Auto-fill...", AUTO_FILL_DIR_MSG, -1, false, !IsFiltered() ? MF_ENABLED : MF_GRAYED);
	AddToMenu(hAutoFillSubMenu, "Auto-fill from default resource path", AUTO_FILL_DEFAULT_MSG, -1, false, !IsFiltered() ? MF_ENABLED : MF_GRAYED);
	AddToMenu(hAutoFillSubMenu, "Auto-fill from project path", AUTO_FILL_PRJ_MSG, -1, false, !IsFiltered() ? MF_ENABLED : MF_GRAYED);
	if (g_type == SNM_SLOT_PRJ)
		AddToMenu(hAutoFillSubMenu, "Auto-fill with recent projects", PRJ_AUTOFILL_RECENTS_MSG, -1, false, !IsFiltered() ? MF_ENABLED : MF_GRAYED);

	if (GetCurList()->HasAutoSave())
	{
		AddToMenu(hMenu, SWS_SEPARATOR, 0);

		HMENU hAutoSaveSubMenu = CreatePopupMenu();
		AddToMenu(hMenu, "Auto-save", AUTOSAVE_MSG, -1, false, !IsFiltered() ? MF_ENABLED : MF_GRAYED);
		AddSubMenu(hMenu, hAutoSaveSubMenu, "Auto-save configuration");
		char autoSavePath[BUFFER_SIZE] = "";
		_snprintf(autoSavePath, BUFFER_SIZE, "(Current auto-save path: %s)", GetCurAutoSaveDir()->Get());
		AddToMenu(hAutoSaveSubMenu, autoSavePath, 0, -1, false, MF_DISABLED); // different from MFS_DISABLED! more readable (REAPER like)
		AddToMenu(hAutoSaveSubMenu, "Set auto-save directory...", AUTOSAVE_DIR_MSG);
		AddToMenu(hAutoSaveSubMenu, "Set auto-save directory to default resource path", AUTOSAVE_DIR_DEFAULT_MSG);
		switch(g_type)
		{
			case SNM_SLOT_FXC:
				AddToMenu(hAutoSaveSubMenu, "Set auto-save directory to project path (/FXChains)", AUTOSAVE_DIR_PRJ_MSG);
				AddToMenu(hAutoSaveSubMenu, SWS_SEPARATOR, 0);
				AddToMenu(hAutoSaveSubMenu, "Auto-save FX chains from track selection", FXC_AUTOSAVE_TR_MSG, -1, false, m_autoSaveFXChainPref == FXC_AUTOSAVE_PREF_TRACK ? MFS_CHECKED : MFS_UNCHECKED);
				AddToMenu(hAutoSaveSubMenu, "Auto-save FX chains from item selection", FXC_AUTOSAVE_ITEM_MSG, -1, false, m_autoSaveFXChainPref == FXC_AUTOSAVE_PREF_ITEM ? MFS_CHECKED : MFS_UNCHECKED);
				if (g_bv4)
					AddToMenu(hAutoSaveSubMenu, "Auto-save input FX chains from track selection", FXC_AUTOSAVE_INPUT_FX, -1, false, m_autoSaveFXChainPref == FXC_AUTOSAVE_PREF_INPUT_FX ? MFS_CHECKED : MFS_UNCHECKED);
				break;
			case SNM_SLOT_TR:
				AddToMenu(hAutoSaveSubMenu, "Set auto-save directory to project path (/TrackTemplates)", AUTOSAVE_DIR_PRJ_MSG);
				AddToMenu(hAutoSaveSubMenu, SWS_SEPARATOR, 0);
				AddToMenu(hAutoSaveSubMenu, "Auto-save track templates with items", TRT_AUTOSAVE_WITEMS_MSG, -1, false, m_autoSaveTrTmpltWithItemsPref ? MFS_CHECKED : MFS_UNCHECKED);
				break;
			case SNM_SLOT_PRJ:
				AddToMenu(hAutoSaveSubMenu, "Set auto-save directory to project path (/ProjectTemplates)", AUTOSAVE_DIR_PRJ_MSG);
				break;
		}
	}

	AddToMenu(hMenu, SWS_SEPARATOR, 0);
	HMENU hFilterSubMenu = CreatePopupMenu();
	AddSubMenu(hMenu, hFilterSubMenu, "Filter on");
	AddToMenu(hFilterSubMenu, "Name", FILTER_BY_NAME_MSG, -1, false, (g_filterPref&1) ? MFS_CHECKED : MFS_UNCHECKED);
	AddToMenu(hFilterSubMenu, "Path", FILTER_BY_PATH_MSG, -1, false, (g_filterPref&2) ? MFS_CHECKED : MFS_UNCHECKED);
	AddToMenu(hFilterSubMenu, "Comment", FILTER_BY_COMMENT_MSG, -1, false, (g_filterPref&4) ? MFS_CHECKED : MFS_UNCHECKED);
	return hMenu;
}

void SNM_ResourceWnd::OnDestroy() 
{
	// save prefs
	char cTmp[8];
	_snprintf(cTmp, 8, "%d", m_cbType.GetCurSel());
	WritePrivateProfileString("RESOURCE_VIEW", "Type", cTmp, g_SNMIniFn.Get());
	_snprintf(cTmp, 8, "%d", g_filterPref);
	WritePrivateProfileString("RESOURCE_VIEW", "FilterByPath", cTmp, g_SNMIniFn.Get()); 

	_snprintf(cTmp, 8, "%d", g_dblClickTo);
	WritePrivateProfileString("RESOURCE_VIEW", "DblClick_To", cTmp, g_SNMIniFn.Get()); 
	_snprintf(cTmp, 8, "%d", m_autoSaveFXChainPref);
	WritePrivateProfileString("RESOURCE_VIEW", "AutoSaveFXChain", cTmp, g_SNMIniFn.Get()); 

	WritePrivateProfileString("RESOURCE_VIEW", "AutoSaveTrTemplateWithItems", m_autoSaveTrTmpltWithItemsPref ? "1" : "0", g_SNMIniFn.Get()); 

	_snprintf(cTmp, 8, "%d", g_prjLoaderStartPref);
	WritePrivateProfileString("RESOURCE_VIEW", "ProjectLoaderStartSlot", cTmp, g_SNMIniFn.Get()); 
	_snprintf(cTmp, 8, "%d", g_prjLoaderEndPref);
	WritePrivateProfileString("RESOURCE_VIEW", "ProjectLoaderEndSlot", cTmp, g_SNMIniFn.Get()); 

	// auto-save, auto-fill directories, etc..
	char iniKey[64]=""; WDL_FastString escapedStr;
	for (int i=0; i < g_slots.GetSize(); i++)
	{
		if (g_slots.Get(i)->HasAutoSave()) {
			_snprintf(iniKey, 64, "AutoSaveDir%s", g_slots.Get(i)->GetResourceDir(false));
			makeEscapedConfigString(g_autoSaveDirs.Get(i)->Get(), &escapedStr);
			WritePrivateProfileString("RESOURCE_VIEW", iniKey, escapedStr.Get(), g_SNMIniFn.Get()); 
		}
		_snprintf(iniKey, 64, "AutoFillDir%s", g_slots.Get(i)->GetResourceDir(false));
		makeEscapedConfigString(g_autoFillDirs.Get(i)->Get(), &escapedStr);
		WritePrivateProfileString("RESOURCE_VIEW", iniKey, escapedStr.Get(), g_SNMIniFn.Get());
		if (g_slots.Get(i)->HasDblClick()) {
			_snprintf(iniKey, 64, "DblClick%s", g_slots.Get(i)->GetResourceDir(false));
			_snprintf(cTmp, 8, "%d", g_dblClickType[i]);
			WritePrivateProfileString("RESOURCE_VIEW", iniKey, cTmp, g_SNMIniFn.Get());
		}
	}

	m_cbType.Empty();
	m_cbDblClickType.Empty();
	m_cbDblClickTo.Empty();
}


int SNM_ResourceWnd::OnKey(MSG* _msg, int _iKeyState) 
{
	if (_msg->message == WM_KEYDOWN)
	{
		// F2 (no modifier)
		if (_msg->wParam == VK_F2 && !_iKeyState)
		{
			OnCommand(RENAME_MSG, 0);
			return 1;
		}
		// DEL (no modifier)
		else if (_msg->wParam == VK_DELETE && !_iKeyState) {
			ClearDeleteSelectedSlots(3, true);
			return 1;
		}
	}
	return 0;
}

int SNM_ResourceWnd::GetValidDroppedFilesCount(HDROP _h)
{
	int validCnt=0;
	int iFiles = DragQueryFile(_h, 0xFFFFFFFF, NULL, 0);
	char cFile[BUFFER_SIZE];
	for (int i = 0; i < iFiles; i++) 
	{
		DragQueryFile(_h, i, cFile, BUFFER_SIZE);
		if (!strcmp(cFile, DRAGDROP_EMPTY_SLOT) || GetCurList()->IsValidFileExt(GetFileExtension(cFile)))
			validCnt++;
	}
	return validCnt;
}

void SNM_ResourceWnd::OnDroppedFiles(HDROP _h)
{
	int dropped = 0; //nb of successfully dropped files
	int iFiles = DragQueryFile(_h, 0xFFFFFFFF, NULL, 0);
	int iValidFiles = GetValidDroppedFilesCount(_h);

	// Check to see if we dropped on a group
	POINT pt;
	DragQueryPoint(_h, &pt);

	RECT r; // ClientToScreen doesn't work right, wtf?
	GetWindowRect(m_hwnd, &r);
	pt.x += r.left;
	pt.y += r.top;

	PathSlotItem* pItem = (PathSlotItem*)m_pLists.Get(0)->GetHitItem(pt.x, pt.y, NULL);
	int dropSlot = GetCurList()->Find(pItem);

	// internal d'n'd ?
	if (g_dragPathSlotItems.GetSize())
	{
		int srcSlot = GetCurList()->Find(g_dragPathSlotItems.Get(0));
		// drag'n'drop slot to the bottom
		if (srcSlot >= 0 && srcSlot < dropSlot)
			dropSlot++; // d'n'd will be more 'natural'
	}

	// drop but not on a slot => create slots
	if (!pItem || dropSlot < 0 || dropSlot >= GetCurList()->GetSize()) 
	{
		dropSlot = GetCurList()->GetSize();
		for (int i = 0; i < iValidFiles; i++)
			GetCurList()->Add(new PathSlotItem());
	}
	// drop on a slot => insert need slots at drop point
	else 
	{
		for (int i = 0; i < iValidFiles; i++)
			GetCurList()->InsertSlot(dropSlot);
	}

	// re-sync pItem 
	pItem = GetCurList()->Get(dropSlot); 

	// Patch added/inserted slots from dropped data
	char cFile[BUFFER_SIZE];
	int slot;
	for (int i = 0; pItem && i < iFiles; i++)
	{
		slot = GetCurList()->Find(pItem);
		DragQueryFile(_h, i, cFile, BUFFER_SIZE);

		// internal d'n'd ?
		if (g_dragPathSlotItems.GetSize())
		{
			PathSlotItem* item = GetCurList()->Get(slot);
			if (item)
			{
				item->m_shortPath.Set(g_dragPathSlotItems.Get(i)->m_shortPath.Get());
				item->m_comment.Set(g_dragPathSlotItems.Get(i)->m_comment.Get());
				dropped++;
				pItem = GetCurList()->Get(slot+1); 
			}
		}
		// .rfxchain? .rTrackTemplate? etc..
		else if (GetCurList()->IsValidFileExt(GetFileExtension(cFile))) {
			if (GetCurList()->SetFromFullPath(slot, cFile)) {
				dropped++;
				pItem = GetCurList()->Get(slot+1); 
			}
		}
	}

	if (dropped)
	{
		// internal drag'n'drop: move (=> delete previous slots)
		for (int i=0; i < g_dragPathSlotItems.GetSize(); i++)
			for (int j=GetCurList()->GetSize()-1; j >= 0; j--)
				if (GetCurList()->Get(j) == g_dragPathSlotItems.Get(i)) {
					if (j < dropSlot) dropSlot--;
					GetCurList()->Delete(j, false);
				}

		Update();

		// Select item at drop point
		if (dropSlot >= 0)
			SelectBySlot(dropSlot);
	}

	g_dragPathSlotItems.Empty(false);
	DragFinish(_h);
}

void SNM_ResourceWnd::DrawControls(LICE_IBitmap* _bm, const RECT* _r, int* _tooltipHeight)
{
	int x0=_r->left+10, h=35;
	if (_tooltipHeight)
		*_tooltipHeight = h;

	LICE_CachedFont* font = SNM_GetThemeFont();
	IconTheme* it = (IconTheme*)GetIconThemeStruct(NULL);// returns the whole icon theme (icontheme.h) and the size

	// defines a new rect 'r' that takes the filter edit box into account (contrary to '_r')
	RECT r;
	GetWindowRect(GetDlgItem(m_hwnd, IDC_FILTER), &r);
	ScreenToClient(m_hwnd, (LPPOINT)&r);
	ScreenToClient(m_hwnd, ((LPPOINT)&r)+1);
	r.top = _r->top; r.bottom = _r->bottom;
	r.right = r.left; r.left = _r->left;

	// "auto-save" button
	m_btnAutoSave.SetGrayed(!GetCurList()->HasAutoSave());
	WDL_VirtualIconButton_SkinConfig* skin = it ? &(it->toolbar_save) : NULL;
	if (skin)
		m_btnAutoSave.SetIcon(skin);
	else {
		m_btnAutoSave.SetTextLabel("Auto-save", 0, font);
		m_btnAutoSave.SetForceBorder(true);
	}
	if (!SNM_AutoVWndPosition(&m_btnAutoSave, NULL, &r, &x0, _r->top, h))
		return;

	// type dropdown (common)
	m_cbType.SetFont(font);
	if (!SNM_AutoVWndPosition(&m_cbType, NULL, &r, &x0, _r->top, h))
		return;

	// "dbl-click to" (common)
	m_txtDblClickType.SetVisible(GetCurList()->HasDblClick());
	m_cbDblClickType.SetVisible(GetCurList()->HasDblClick());
	if (GetCurList()->HasDblClick())
	{
		m_txtDblClickType.SetFont(font);
		if (!SNM_AutoVWndPosition(&m_txtDblClickType, NULL, &r, &x0, _r->top, h, 5))
			return;
		m_cbDblClickType.SetFont(font);
		if (!SNM_AutoVWndPosition(&m_cbDblClickType, &m_txtDblClickType, &r, &x0, _r->top, h))
			return;
	}

	// "to selected" (fx chain only)
	m_txtDblClickTo.SetVisible(g_type == SNM_SLOT_FXC);
	m_cbDblClickTo.SetVisible(g_type == SNM_SLOT_FXC);
	if (g_type == SNM_SLOT_FXC)
	{
		m_txtDblClickTo.SetFont(font);
		if (!SNM_AutoVWndPosition(&m_txtDblClickTo, NULL, &r, &x0, _r->top, h, 5))
			return;
		m_cbDblClickTo.SetFont(font);
		if (!SNM_AutoVWndPosition(&m_cbDblClickTo, &m_txtDblClickTo, &r, &x0, _r->top, h))
			return;
	}

/*JFB MFC filter edit box instead
	SNM_AddLogo(_bm, _r, x0, h);
*/
}

HBRUSH SNM_ResourceWnd::OnColorEdit(HWND _hwnd, HDC _hdc)
{
	int bg, txt; bool match=false;
	if (_hwnd == GetDlgItem(m_hwnd, IDC_EDIT))
	{
		match = true;
		SNM_GetThemeListColors(&bg, &txt);
	}
	else if (_hwnd == GetDlgItem(m_hwnd, IDC_FILTER))
	{
		match = true;
		SNM_GetThemeEditColors(&bg, &txt);
	}
	if (match)
	{
		SetBkColor(_hdc, bg);
		SetTextColor(_hdc, txt);
		return SNM_GetThemeBrush(bg);
	}
	return 0;
}

void SNM_ResourceWnd::ClearListSelection()
{
	SWS_ListView* lv = m_pLists.Get(0);
	HWND hList = lv ? lv->GetHWND() : NULL;
	if (hList) // can be called when the view is closed!
		ListView_SetItemState(hList, -1, 0, LVIS_SELECTED);
}

// select a range of slots or a single slot (when _slot2 < 0)
void SNM_ResourceWnd::SelectBySlot(int _slot1, int _slot2)
{
	SWS_ListView* lv = m_pLists.Get(0);
	HWND hList = lv ? lv->GetHWND() : NULL;
	if (lv && hList) // can be called when the view is closed!
	{
		int slot1=_slot1, slot2=_slot2;

		// check params
		if (_slot1 < 0)
			return;
		if (_slot2 < 0)
			slot2 = slot1;
		else if (_slot2 < _slot1) {
			slot1 = _slot2;
			slot2 = _slot1;
		}

		ListView_SetItemState(hList, -1, 0, LVIS_SELECTED);

		int firstSel = -1;
		for (int i = 0; i < lv->GetListItemCount(); i++)
		{
			PathSlotItem* item = (PathSlotItem*)lv->GetListItem(i);
			int slot = GetCurList()->Find(item);
			if (item && slot >= _slot1 && slot <= slot2) 
			{
				if (firstSel < 0)
					firstSel = i;
				ListView_SetItemState(hList, i, LVIS_SELECTED, LVIS_SELECTED);
				if (_slot2 < 0) // one slot to be selected?
					break;
			}
		}
		if (firstSel >= 0)
			ListView_EnsureVisible(hList, firstSel, true);
	}
}

void SNM_ResourceWnd::AddSlot(bool _update)
{
	int idx = GetCurList()->GetSize();
	if (GetCurList()->Add(new PathSlotItem()) && _update) {
		Update();
		SelectBySlot(idx);
	}
}

void SNM_ResourceWnd::InsertAtSelectedSlot(bool _update)
{
	if (GetCurList()->GetSize())
	{
		bool updt = false;
		PathSlotItem* item = (PathSlotItem*)m_pLists.Get(0)->EnumSelected(NULL);
		if (item)
		{
			int slot = GetCurList()->Find(item);
			if (slot >= 0)
				updt = (GetCurList()->InsertSlot(slot) != NULL);
			if (_update && updt) 
			{
				Update();
				SelectBySlot(slot);
				return; // <-- !!
			}
		}
	}
	AddSlot(_update); // empty list, no selection, etc.. => add
}

// _mode: _mode&1 = delete slots (clear otherwise), _mode&2 = delete only *last* empty slots, _mode&4 = delete files
// note: _mode&2 is just an option to avoid leaving empty slots at the end of the list when clearing
void SNM_ResourceWnd::ClearDeleteSelectedSlots(int _mode, bool _update)
{
	bool updt = false;
	int oldSz = GetCurList()->GetSize();

	WDL_PtrList<int> selSlots;
	while (selSlots.GetSize() != GetCurList()->GetSize())
		selSlots.Add(&g_i0);

	int x=0;
	while(PathSlotItem* item = (PathSlotItem*)m_pLists.Get(0)->EnumSelected(&x)) {
		int slot = GetCurList()->Find(item);
		selSlots.Delete(slot);
		selSlots.Insert(slot, &g_i1);
	}

	int endSelSlot=-1;
	x=GetCurList()->GetSize();
	while(x >= 0 && selSlots.Get(--x) == &g_i1) endSelSlot=x;

	char fullPath[BUFFER_SIZE] = "";
	WDL_PtrList_DeleteOnDestroy<PathSlotItem> delItems; // DeleteOnDestroy: keep (displayed!) pointers until the list view is updated
	for (int slot=selSlots.GetSize()-1; slot >=0 ; slot--)
	{
		if (*selSlots.Get(slot)) // avoids new Find(item) calls! 
		{
			if (PathSlotItem* item = GetCurList()->Get(slot))
			{
				if (_mode&4) {
					GetFullResourcePath(GetCurList()->GetResourceDir(), item->m_shortPath.Get(), fullPath, BUFFER_SIZE);
					SNM_DeleteFile(fullPath, true);
				}

				if (_mode&1 || (_mode&2 && endSelSlot>=0 && slot>=endSelSlot)) 
				{
					selSlots.Delete(slot, false); // keep the sel list "in sync"
					GetCurList()->Delete(slot, false); // remove slot - but no deleted pointer yet
					delItems.Add(item); // for later pointer deletion..
				}
				else
					GetCurList()->ClearSlot(slot, false);
				updt = true;
			}
		}
	}

	if (_update && updt)
	{
		Update();
		if (oldSz != GetCurList()->GetSize()) // deletion?
			ClearListSelection();
	}

} // + delItems auto clean-up !

void SNM_ResourceWnd::AutoSave()
{
	if (!FileExists(GetCurAutoSaveDir()->Get()))
	{
		char msg[BUFFER_SIZE] = "No auto-save directory defined.\nDo you want to select one ?";
		if (GetCurAutoSaveDir()->GetLength()) // e.g. "My Computer" on windows
			_snprintf(msg, BUFFER_SIZE, "Auto-save directory not found:\n%s\n\nDo you want to select one ?", GetCurAutoSaveDir()->Get());
		switch(MessageBox(g_hwndParent, msg, "S&M - Error", MB_YESNO)) {
			case IDYES: this->OnCommand(AUTOSAVE_DIR_MSG, 0); break;
			case IDNO: return;
		}
		if (!FileExists(GetCurAutoSaveDir()->Get())) // re-check for cancel..
			return;
	}

	int slotStart = GetCurList()->GetSize();
	char fn[BUFFER_SIZE] = "";
	switch(g_type) 
	{
		case SNM_SLOT_FXC:
			switch (m_autoSaveFXChainPref)
			{
				case FXC_AUTOSAVE_PREF_INPUT_FX:
					autoSaveTrackFXChainSlots(g_bv4, GetCurAutoSaveDir()->Get(), fn, BUFFER_SIZE);
					break;
				case FXC_AUTOSAVE_PREF_TRACK:
					autoSaveTrackFXChainSlots(false, GetCurAutoSaveDir()->Get(), fn, BUFFER_SIZE);
					break;
				case FXC_AUTOSAVE_PREF_ITEM:
					autoSaveItemFXChainSlots(GetCurAutoSaveDir()->Get(), fn, BUFFER_SIZE);
					break;
			}
			break;
		case SNM_SLOT_TR:
			autoSaveTrackSlots(!m_autoSaveTrTmpltWithItemsPref, true /*JFB? manage envelopes?*/, GetCurAutoSaveDir()->Get(), fn, BUFFER_SIZE);
			break;
		case SNM_SLOT_PRJ:
			autoSaveProjectSlot(true, GetCurAutoSaveDir()->Get(), fn, BUFFER_SIZE);
			break;
		case SNM_SLOT_MEDIA:
			autoSaveMediaSlot(GetCurAutoSaveDir()->Get(), fn, BUFFER_SIZE);
			break;
	}

	if (slotStart != GetCurList()->GetSize())
	{
		Update();
		SelectBySlot(slotStart, GetCurList()->GetSize());
	}
	else
	{
		char msg[BUFFER_SIZE];
		if (fn && *fn) _snprintf(msg, BUFFER_SIZE, "Auto-save failed: %s\n%s", fn, AUTO_SAVE_ERR_STR);
		else _snprintf(msg, BUFFER_SIZE, "Auto-save failed!\n%s", AUTO_SAVE_ERR_STR);
		MessageBox(g_hwndParent, msg, "S&M - Error", MB_OK);
	}
}

void SNM_ResourceWnd::AutoFill(const char* _startPath)
{
	int startSlot = GetCurList()->GetSize();
	if (_startPath && *_startPath)
	{
		WDL_PtrList_DeleteOnDestroy<WDL_String> files; 
		ScanFiles(&files, _startPath, GetCurList()->GetFileExt(), true);
		if (int sz = files.GetSize())
			for (int i=0; i < sz; i++)				
				if (GetCurList()->FindByResFulltPath(files.Get(i)->Get()) < 0) // skip if already present
					GetCurList()->AddSlot(files.Get(i)->Get());
	}
	if (startSlot != GetCurList()->GetSize())
	{
		Update();
		SelectBySlot(startSlot, GetCurList()->GetSize());
	}
	else
	{
		char msg[BUFFER_SIZE*2];
		if (_startPath && *_startPath) _snprintf(msg, BUFFER_SIZE*2, "No slot added from: %s\n%s", _startPath, AUTO_FILL_ERR_STR);
		else _snprintf(msg, BUFFER_SIZE, "No slot added!\n%s", AUTO_FILL_ERR_STR);
		MessageBox(GetMainHwnd(), msg, "S&M - Warning", MB_OK);
	}
}


///////////////////////////////////////////////////////////////////////////////

void readSlotIniFile(const char* _key, int _slot, char* _path, int _pathSize, char* _desc, int _descSize)
{
	char buf[32];
	_snprintf(buf, 32, "SLOT%d", _slot+1);
	GetPrivateProfileString(_key, buf, "", _path, _pathSize, g_SNMIniFn.Get());
	_snprintf(buf, 32, "DESC%d", _slot+1);
	GetPrivateProfileString(_key, buf, "", _desc, _descSize, g_SNMIniFn.Get());
}

// adds custom slot types from the S&M.ini file, example: 
// CustomSlotType1="Data\track_icons,track icons,png"
// CustomSlotType2="TextFiles,text file,txt"
void readCustomTypesIniFile(int _initialType)
{
	int i=0; char buf[BUFFER_SIZE]=""; WDL_String iniKeyStr("CustomSlotType1"), tokenStrs[3];
	GetPrivateProfileString("RESOURCE_VIEW", iniKeyStr.Get(), "", buf, BUFFER_SIZE, g_SNMIniFn.Get());
	while (*buf && i < SNM_MAX_SLOT_TYPES)
	{
		if (char* tok = strtok(buf, ",")) {
			if (tok[strlen(tok)-1] == PATH_SLASH_CHAR) tok[strlen(tok)-1] = '\0';
			tokenStrs[0].Set(tok);
			if (tok = strtok(NULL, ",")) {
				tokenStrs[1].Set(tok);
				if (tok = strtok(NULL, ",")) {
					tokenStrs[2].Set(tok);
					g_slots.Add(new FileSlotList(_initialType++, tokenStrs[0].Get(), tokenStrs[1].Get(), tokenStrs[2].Get(), false, false, false));
				}
			}
		}
		iniKeyStr.SetFormatted(32, "CustomSlotType%d", (++i)+1);
		GetPrivateProfileString("RESOURCE_VIEW", iniKeyStr.Get(), "", buf, BUFFER_SIZE, g_SNMIniFn.Get());
	}
}


// JFB add new resource types here ------------------------------------------->
void InitLists() 
{
	int type=0;
	g_slots.Empty(true);
	g_slots.Add(new FileSlotList(type++, "FXChains", "FX chain", "RfxChain", true, true, true));
	g_slots.Add(new FileSlotList(type++, "TrackTemplates", "track template", "RTrackTemplate", true, true, true));
	g_slots.Add(new FileSlotList(type++, "ProjectTemplates", "project template", "RPP", true, true, true));
	g_slots.Add(new FileSlotList(type++, "MediaFiles", "media file", "", false, true, true)); // "" means "all supported media files"
	// etc..

#ifdef _WIN32
	// keep these as the last ones (win only)
	g_slots.Add(new FileSlotList(type++, "ColorThemes", "theme", "ReaperthemeZip", false, false, true)); 
	readCustomTypesIniFile(type);
#endif
}
// <---------------------------------------------------------------------------


int ResourceViewInit()
{
	InitLists(); // lists ordered by g_type

	// read slots from ini file
	char path[BUFFER_SIZE] = "";
	char desc[128], maxSlotCount[16];
	for (int i=0; i < g_slots.GetSize(); i++)
	{
		if (FileSlotList* list = g_slots.Get(i))
		{
			GetPrivateProfileString(list->GetResourceDir(false), "MAX_SLOT", "0", maxSlotCount, 16, g_SNMIniFn.Get()); 
			list->EmptySafe(true);
			int slotCount = atoi(maxSlotCount);
			for (int j=0; j < slotCount; j++) {
				readSlotIniFile(list->GetResourceDir(false), j, path, BUFFER_SIZE, desc, 128);
				list->Add(new PathSlotItem(path, desc));
			}
		}
	}

	// instanciate the view
	g_pResourcesWnd = new SNM_ResourceWnd();
	if (!g_pResourcesWnd)
		return 0;
	return 1;
}

void ResourceViewExit()
{
	// save slots to ini file
	for (int i=0; i < g_slots.GetSize(); i++)
	{
		if (FileSlotList* list = g_slots.Get(i))
		{
			WDL_FastString iniSection;
			iniSection.SetFormatted(32, "MAX_SLOT=%d\n", list->GetSize());
			for (int j=0; j < list->GetSize(); j++)
			{
				if (PathSlotItem* item = list->Get(j))
				{
					WDL_FastString escapedStr;
					if (item->m_shortPath.GetLength()) {
						makeEscapedConfigString(item->m_shortPath.Get(), &escapedStr);
						iniSection.AppendFormatted(BUFFER_SIZE, "SLOT%d=%s\n", j+1, escapedStr.Get()); 
					}
					if (item->m_comment.GetLength()) {
						makeEscapedConfigString(item->m_comment.Get(), &escapedStr);
						iniSection.AppendFormatted(BUFFER_SIZE, "DESC%d=%s\n", j+1, escapedStr.Get());
					}
				}
			}
			// write things in one go (avoid to slow down REAPER shutdown)
			SaveIniSection(list->GetResourceDir(false), &iniSection, g_SNMIniFn.Get());
		}
	}
	DELETE_NULL(g_pResourcesWnd);
}

void OpenResourceView(COMMAND_T* _ct) 
{
	if (g_pResourcesWnd) 
	{
		int prevType = g_type;
		if (g_type < 0)
			g_type = (int)_ct->user;
		g_pResourcesWnd->Show((prevType == (int)_ct->user) /* i.e toggle */, true);
		g_pResourcesWnd->SetType((int)_ct->user);
//		SetFocus(GetDlgItem(g_pResourcesWnd->GetHWND(), IDC_FILTER));
	}
}

bool IsResourceViewDisplayed(COMMAND_T* _ct) {
	return (g_pResourcesWnd && g_pResourcesWnd->IsValidWindow());
}

void ResourceViewClearSlotPrompt(COMMAND_T* _ct) {
	g_slots.Get((int)_ct->user)->ClearSlotPrompt(_ct);
}

void ResourceViewAutoSave(COMMAND_T* _ct) {
	if (g_slots.Get((int)_ct->user)->HasAutoSave()) {
		//JFB TO DO?? (requires serious refactoring..)
	}
}
