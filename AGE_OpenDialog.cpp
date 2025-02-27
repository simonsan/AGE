#include "AGE_OpenDialog.h"

AGE_OpenDialog::AGE_OpenDialog(wxWindow *parent, const wxFont &font)
: AGE_OpenSave(parent, "Open", this, font)
{
    SolidText *TerrainsText = new SolidText(this, "      Terrains: ");
    TerrainsBox = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, -1));
    Path_DatFileLocation = new wxFilePickerCtrl(this, wxID_ANY, "", "Select a file", "Compressed data set (*.dat)|*.dat", wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_USE_TEXTCTRL | wxFLP_FILE_MUST_EXIST);
    Button_RawDecompress = new wxButton(this, wxID_ANY, "Decompress only", wxDefaultPosition, wxSize(5, -1));
    Path_RawDecompress = new wxFilePickerCtrl(this, wxID_ANY, "", "Select a file", "Compressed genie file|*", wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_USE_TEXTCTRL | wxFLP_FILE_MUST_EXIST);

    Path_LangFileLocation = new wxFilePickerCtrl(this, wxID_ANY, "", "Select a file", "DLL or text (*.dll, *.txt)|*.dll;*.txt", wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_USE_TEXTCTRL | wxFLP_FILE_MUST_EXIST);
    Path_LangX1FileLocation = new wxFilePickerCtrl(this, wxID_ANY, "", "Select a file", "DLL or text (*.dll, *.txt)|*.dll;*.txt", wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_USE_TEXTCTRL | wxFLP_FILE_MUST_EXIST);
    Path_LangX1P1FileLocation = new wxFilePickerCtrl(this, wxID_ANY, "", "Select a file", "DLL or text (*.dll, *.txt)|*.dll;*.txt", wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_USE_TEXTCTRL | wxFLP_FILE_MUST_EXIST);
    CheckBox_LangWrite = new wxCheckBox(this, wxID_ANY, "Write language files *");
    CheckBox_LangWrite->SetToolTip("WARNING! This feature is still experimental\nand affects reading too");
    CheckBox_LangWrite->SetBackgroundColour(wxColour(240, 200, 200));
    CheckBox_LangWriteToLatest = new wxCheckBox(this, wxID_ANY, "Write to the latest file instead of the base file *      Google Resource Hacker or Resource Tuner");
    CheckBox_LangWriteToLatest->SetToolTip("If you write to the latest file (x1/p1),\nyou only need to distribute that file,\nbut your edits will not affect the vanilla game");
    CheckBox_DRSPath = new AGE_PairedCheckBox(this, "Path for DRS files *", (wxWindow**)&Path_DRS);
    CheckBox_DRSPath->SetToolTip("Path to the folder containing all drs files/folders");
    CheckBox_DRSPath2 = new AGE_PairedCheckBox(this, "Path for mod DRS files", (wxWindow**)&Path_DRS2);
    CheckBox_DRSPath3 = new AGE_PairedCheckBox(this, "Extra 1st priority DRS file", (wxWindow**)&Path_DRS3);
    CheckBox_SlpPath = new AGE_PairedCheckBox(this, "Path for loose SLP files", (wxWindow**)&Path_SLP);
    Path_DRS = new wxDirPickerCtrl(this, wxID_ANY, "", "Select a folder", wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST);
    Path_DRS2 = new wxDirPickerCtrl(this, wxID_ANY, "", "Select a folder", wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST);
    Path_DRS3 = new wxFilePickerCtrl(this, wxID_ANY, "", "Select a file", "DRS (*.drs)|*.drs", wxDefaultPosition, wxDefaultSize, wxFLP_OPEN | wxFLP_USE_TEXTCTRL | wxFLP_FILE_MUST_EXIST);
    Path_SLP = new wxDirPickerCtrl(this, wxID_ANY, "", "Select a folder", wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST);

    Extras->Add(TerrainsText);
    Extras->Add(TerrainsBox);
    Layout->Add(Path_DatFileLocation, 1, wxEXPAND);
    Layout->Add(Button_RawDecompress, 1, wxEXPAND);
    Layout->Add(Path_RawDecompress, 1, wxEXPAND);
    Layout->AddSpacer(15);
    Layout->AddSpacer(15);
    Layout->Add(CheckBox_LangFileLocation, 1, wxEXPAND);
    Layout->Add(Path_LangFileLocation, 1, wxEXPAND);
    Layout->Add(CheckBox_LangX1FileLocation, 1, wxEXPAND);
    Layout->Add(Path_LangX1FileLocation, 1, wxEXPAND);
    Layout->Add(CheckBox_LangX1P1FileLocation, 1, wxEXPAND);
    Layout->Add(Path_LangX1P1FileLocation, 1, wxEXPAND);
    Layout->Add(CheckBox_LangWrite, 1, wxEXPAND);
    Layout->Add(CheckBox_LangWriteToLatest, 1, wxEXPAND);
    Layout->AddSpacer(15);
    Layout->AddSpacer(15);
    Layout->Add(CheckBox_DRSPath, 1, wxEXPAND);
    Layout->Add(Path_DRS, 1, wxEXPAND);
    Layout->Add(CheckBox_DRSPath2, 1, wxEXPAND);
    Layout->Add(Path_DRS2, 1, wxEXPAND);
    Layout->Add(CheckBox_DRSPath3, 1, wxEXPAND);
    Layout->Add(Path_DRS3, 1, wxEXPAND);
    Layout->Add(CheckBox_SlpPath, 1, wxEXPAND);
    Layout->Add(Path_SLP, 1, wxEXPAND);
    Layout->AddSpacer(15);
    Layout->AddSpacer(15);

    Layout->AddGrowableCol(1, 1);
    Layout->AddGrowableRow(17, 1);

    Main->Add(Defaults, 0, wxALIGN_LEFT | wxTOP | wxLEFT | wxRIGHT, 5);
    Main->Add(Defaults_StarWars, 0, wxALIGN_LEFT | wxBOTTOM | wxLEFT | wxRIGHT, 5);
    Main->Add(Layout, 1, wxEXPAND | wxALL, 5);
    Main->Add(Buttons, 0, wxALIGN_RIGHT | wxALL, 5);

    TerrainsBox->Enable(false);
    SetSizerAndFit(Main);

    Button_RawDecompress->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [this](wxCommandEvent&)
    {
        wxString filename = Path_RawDecompress->GetPath();
        wxString savename = filename+"u";
        if(!filename.empty())
        {
            genie::DatFile raw;
            raw.extractRaw(filename.c_str(), savename.c_str());
        }
    });

    ButtonOK->SetDefault();
    ButtonOK->SetFocus();
}

void AGE_OpenDialog::OnDefaultAoE(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultAoE(event);

    Path_DRS->SetPath(game_path + "\\data");
    CheckBox_DRSPath->SetValue(true);
    TerrainsBox->ChangeValue("32");
}

void AGE_OpenDialog::OnDefaultRoR(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultRoR(event);

    Path_DRS->SetPath(game_path + "\\data");
    CheckBox_DRSPath->SetValue(true);
    TerrainsBox->ChangeValue("32");
}

void AGE_OpenDialog::OnDefaultAoK(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultAoK(event);

    Path_DRS->SetPath(game_path + "\\data");
    CheckBox_DRSPath->SetValue(true);
    TerrainsBox->ChangeValue("32");
}

void AGE_OpenDialog::OnDefaultTC(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultTC(event);

    Path_DRS->SetPath(game_path + "\\data");
    CheckBox_DRSPath->SetValue(true);
    TerrainsBox->ChangeValue("42");
}

void AGE_OpenDialog::OnDefaultAoKHD(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultAoKHD(event);

    CheckBox_LangFileLocation->SetValue(true);
    CheckBox_LangX1FileLocation->SetValue(true);
    Path_DRS->SetPath(game_path + "\\resources\\_common\\drs");
    TerrainsBox->ChangeValue("42");
    CheckBox_DRSPath->SetValue(true);
}

void AGE_OpenDialog::OnDefaultAoP(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultAoP(event);

    CheckBox_LangFileLocation->SetValue(true);
    CheckBox_LangX1FileLocation->SetValue(true);
    Path_DRS->SetPath(game_path + "\\resources\\_common\\drs");
    TerrainsBox->ChangeValue("100");
    CheckBox_DRSPath->SetValue(true);
}

void AGE_OpenDialog::OnDefaultDE2(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultDE2(event);

    CheckBox_LangFileLocation->SetValue(true);
    CheckBox_LangX1FileLocation->SetValue(true);
    Path_DRS->SetPath(game_path + "\\resources\\_common\\drs");
    TerrainsBox->ChangeValue("200");
    CheckBox_DRSPath->SetValue(true);
}

void AGE_OpenDialog::OnDefaultSWGB(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultSWGB(event);

    Path_DRS->SetPath(game_path + "\\data");
    CheckBox_DRSPath->SetValue(true);
    TerrainsBox->ChangeValue("55");
}

void AGE_OpenDialog::OnDefaultCC(wxCommandEvent &event)
{
    AGE_OpenSave::OnDefaultCC(event);

    Path_DRS->SetPath(game_path + "\\data");
    CheckBox_DRSPath->SetValue(true);
    TerrainsBox->ChangeValue("55");
}

void AGE_OpenDialog::OnRecent(wxCommandEvent &event)
{
    if(RecentValues.empty()) return;
    int sel = CheckBox_Recent->GetSelection();
    ComboBox_GenieVer->SetSelection(lexical_cast<int>(RecentValues[sel][0]));
    Path_DatFileLocation->SetPath(RecentValues[sel][1]);
    Path_LangFileLocation->SetPath(RecentValues[sel][2]);
    Path_LangX1FileLocation->SetPath(RecentValues[sel][3]);
    Path_LangX1P1FileLocation->SetPath(RecentValues[sel][4]);
    CheckBox_LangFileLocation->SetValue(RecentValues[sel][2].size());
    CheckBox_LangX1FileLocation->SetValue(RecentValues[sel][3].size());
    CheckBox_LangX1P1FileLocation->SetValue(RecentValues[sel][4].size());
    Path_DRS->SetPath(RecentValues[sel][5]);
    Path_DRS2->SetPath(RecentValues[sel][6]);
    Path_DRS3->SetPath(RecentValues[sel][7]);
    Path_SLP->SetPath(RecentValues[sel][8]);
}
