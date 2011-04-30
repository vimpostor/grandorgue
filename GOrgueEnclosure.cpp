/*
 * GrandOrgue - free pipe organ simulator based on MyOrgan
 *
 * MyOrgan 1.0.6 Codebase - Copyright 2006 Milan Digital Audio LLC
 * MyOrgan is a Trademark of Milan Digital Audio LLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include <wx/app.h>
#include "GOrgueEnclosure.h"
#include "GrandOrgueFile.h"
#include "GrandOrgue.h"
#include "MIDIListenDialog.h"
#include "GOrgueDisplayMetrics.h"
#include "IniFileConfig.h"

/* TODO: This should not be... */
extern GOrgueSound* g_sound;
extern char* s_MIDIMessages[];
extern GrandOrgueFile* organfile;

GOrgueEnclosure::GOrgueEnclosure() :
	AmpMinimumLevel(0),
	MIDIInputNumber(0),
	MIDIValue(0),
	Name()
{

}

bool GOrgueEnclosure::Draw(int xx, int yy, wxDC* dc, wxDC* dc2)
{

	int enclosure_x = DisplayMetrics->GetEnclosureX(this);
	int enclosure_y = DisplayMetrics->GetEnclosureY();

	if (!dc)
	{
		wxRect rect(enclosure_x, enclosure_y, 46, 61);
		return rect.Contains(xx, yy);
	}

	dc->SetBrush(*wxBLACK_BRUSH);
	dc->DrawRectangle(enclosure_x, enclosure_y + 13, 46, 44);
	int dx = 1 + ( 3 * MIDIValue) / 127;
	int dy = 1 + (13 * MIDIValue) / 127;
	wxPoint points[4];
	points[0].x = enclosure_x +  7 + dx;
	points[1].x = enclosure_x + 38 - dx;
	points[2].x = enclosure_x + 38 + dx;
	points[3].x = enclosure_x +  7 - dx;
	points[0].y = points[1].y = enclosure_y + 13 + dy;
	points[2].y = points[3].y = enclosure_y + 56 - dy;
	dc->SetBrush(::wxGetApp().frame->m_pedalBrush);
	dc->DrawPolygon(4, points);

	if (dc2)
	{
		dc2->Blit
			(enclosure_x
			,enclosure_y + 13
			,46
			,44
			,dc
			,enclosure_x
			,enclosure_y + 13
			);
	}

	return false;

}

void GOrgueEnclosure::Load(IniFileConfig& cfg, const char* group, GOrgueDisplayMetrics* displayMetrics)
{
	DisplayMetrics = displayMetrics;
	Name=cfg.ReadString( group,"Name",   64);
	AmpMinimumLevel=cfg.ReadInteger( group,"AmpMinimumLevel",    0,  100);
	MIDIInputNumber=cfg.ReadInteger( group,"MIDIInputNumber",    1,    6);
	Set(127);	// default to full volume until we receive any messages
}

void GOrgueEnclosure::Set(int n)
{
	if (n < 0)
		n = 0;
	if (n > 127)
		n = 127;
	MIDIValue = n;
	wxCommandEvent event(wxEVT_ENCLOSURE, 0);
	event.SetClientData(this);
	::wxGetApp().frame->AddPendingEvent(event);
}

void GOrgueEnclosure::MIDI(void)
{
	int index = MIDIInputNumber + 1;
	MIDIListenDialog dlg(::wxGetApp().frame, _(s_MIDIMessages[index]), g_sound->i_midiEvents[index], 0);
	if (dlg.ShowModal() == wxID_OK)
	{
		wxConfigBase::Get()->Write(wxString("MIDI/") + s_MIDIMessages[index], dlg.GetEvent());
		g_sound->ResetSound();
	}
}

int GOrgueEnclosure::GetMIDIInputNumber()
{

	return MIDIInputNumber;

}

float GOrgueEnclosure::GetAttenuation()
{

	static const float scale = 1.0 / 12700.0;
	return (float)(MIDIValue * (100 - AmpMinimumLevel) + 127 * AmpMinimumLevel) * scale;

}

void GOrgueEnclosure::DrawLabel(wxDC& dc)
{

	int enclosure_x = DisplayMetrics->GetEnclosureX(this);
	int enclosure_y = DisplayMetrics->GetEnclosureY();

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);

	wxRect rect(enclosure_x, enclosure_y, 46, 61);
	dc.DrawRectangle(rect.x, rect.y, rect.width, rect.height);

	wxFont font = *wxNORMAL_FONT;
	font.SetPointSize(7);
	dc.SetFont(font);
	dc.SetTextForeground(*wxWHITE);

	dc.DrawLabel(Name, rect, wxALIGN_CENTER_HORIZONTAL);

}

void GOrgueEnclosure::Scroll(bool scroll_up)
{

	Set(MIDIValue + (scroll_up ? 16 : -16));

}
