#include "../AGE_Frame.h"

wxString AGE_Frame::GetEffectName(int index)
{
    if(!dataset->Effects[index].Name.empty())
        return dataset->Effects[index].Name;
    return "New Effect";
}

void AGE_Frame::OnEffectRenameGE2(wxCommandEvent &event)
{
    if(!dataset) return;

    for(auto &tech: dataset->Effects)
    {
        tech.Name = "Effect";
    }
    ListEffects();
}

void AGE_Frame::OnEffectRename(wxCommandEvent &event)
{
    if(!dataset) return;

    for(auto &tech: dataset->Effects)
    {
        if(tech.EffectCommands.size()) // Other than empty techs, not researches if research loop doesn't rename them.
        {
            tech.Name = "Non-Tech";
        }
        else // Empty techs.
        {
            tech.Name = "New Effect";
        }
    }
    short ResearchTechID = 0;
    for(size_t loop=dataset->Techs.size(); loop--> 0;) // Rename of techs. Make it reverse loop.
    {
        ResearchTechID = dataset->Techs[loop].EffectID;
        if(ResearchTechID >= 0) // Only researches which have techs.
        {
            if(!TranslatedText(dataset->Techs[loop].LanguageDLLName, 2).empty()) // has a lang dll name
            {
                dataset->Effects[ResearchTechID].Name = TranslatedText(dataset->Techs[loop].LanguageDLLName, 31);
            }
            else // Use internal name instead.
            {
                dataset->Effects[ResearchTechID].Name = dataset->Techs[loop].Name;
            }
        }
    }
    short CivTechTreeID=0, CivTeamBonusID = 0;
    for(size_t loop2=dataset->Civs.size(); loop2--> 0;) // Rename of techs. Make it reverse loop.
    {
        string CivName = lexical_cast<string>(dataset->Civs[loop2].Name); // Civ internal name.
        CivTechTreeID = dataset->Civs[loop2].TechTreeID;
        CivTeamBonusID = dataset->Civs[loop2].TeamBonusID;
        if(CivTechTreeID >= 0)
        {
            dataset->Effects[CivTechTreeID].Name = CivName+" Tech Tree"; // Under 31 chars.
        }
        if(CivTeamBonusID >= 0)
        {
            dataset->Effects[CivTeamBonusID].Name = CivName+" Team Bonus"; // Under 31 chars.
        }
    }
    ListEffects();
}

void AGE_Frame::OnEffectSearch(wxCommandEvent &event)
{
    How2List = SEARCH;
    ListEffects(false);
}

void AGE_Frame::ListEffects(bool all)
{
    InitTechs(all);
    wxCommandEvent e;
    OnEffectSelect(e);
}

void AGE_Frame::InitTechs(bool all)
{
    InitSearch(Techs_Search->GetValue().MakeLower(), Techs_Search_R->GetValue().MakeLower());
    SearchAnd = Techs_UseAnd[0]->GetValue();
    ExcludeAnd = Techs_UseAnd[1]->GetValue();

    Techs_ListV->Sweep();
    if(all)
    {
        tech_names.Clear();
        tech_names.Alloc(1 + dataset->Effects.size());
        tech_names.Add("-1 - None");
    }

    for(size_t loop = 0; loop < dataset->Effects.size(); ++loop)
    {
        wxString Name = FormatInt(loop)+" - "+GetEffectName(loop);
        if(SearchMatches(" " + Name.Lower() + " "))
        {
            Techs_ListV->names.Add(Name);
            Techs_ListV->indexes.push_back(loop);
        }
        if(all) tech_names.Add(Name);
    }

    RefreshList(Techs_ListV, &TechIDs);
    if(all) for(auto &list: TechComboBoxList) list->Flash();

    SearchAnd = ExcludeAnd = false;
}

void AGE_Frame::OnEffectSelect(wxCommandEvent &event)
{
    auto selections = Techs_ListV->GetSelectedCount();
    wxBusyCursor WaitCursor;
    getSelectedItems(selections, Techs_ListV, TechIDs);

    Techs_Name->clear();

    genie::Effect * TechPointer;
    for(auto loop = selections; loop--> 0;)
    {
        TechPointer = &dataset->Effects[TechIDs[loop]];
        Techs_Name->prepend(&TechPointer->Name);
    }
    SetStatusText("Selections: "+lexical_cast<string>(selections)+"    Selected effect: "+lexical_cast<string>(TechIDs.front()), 0);

    Techs_Name->update();
    ListEffectCmds();
}

void AGE_Frame::OnEffectAdd(wxCommandEvent &event)    // Works.
{
    if(!dataset) return;

    wxBusyCursor WaitCursor;
    AddToList(dataset->Effects);
    ListEffects();
}

void AGE_Frame::OnEffectInsert(wxCommandEvent &event) // Works.
{
    auto selections = Techs_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    InsertToList(dataset->Effects, TechIDs.front());
    ListEffects();
}

void AGE_Frame::OnEffectDelete(wxCommandEvent &event) // Works.
{
    auto selections = Techs_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    DeleteFromList(dataset->Effects, TechIDs);
    ListEffects();
}

void AGE_Frame::OnEffectCopy(wxCommandEvent &event)   // Works.
{
    auto selections = Techs_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    CopyFromList(dataset->Effects, TechIDs, copies.Effect);
    Techs_ListV->SetFocus();
}

void AGE_Frame::OnEffectPaste(wxCommandEvent &event)  // Works.
{
    auto selections = Techs_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    PasteToList(dataset->Effects, TechIDs, copies.Effect);
    ListEffects();
}

void AGE_Frame::OnEffectPasteInsert(wxCommandEvent &event)    // Works.
{
    auto selections = Techs_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    PasteInsertToList(dataset->Effects, TechIDs.front(), copies.Effect);
    ListEffects();
}

wxString AGE_Frame::Tester(genie::EffectCommand effect, wxString how)
{
    return ((effect.C == 8 || effect.C == 9) ? (effect.C == 8 ? "armor type " : "attack type ")
        + FormatInt((uint16_t)effect.D >> 8) + how + FormatInt(uint16_t(effect.D) & 0xFF)
        : "attr " + FormatInt(effect.C) + how + FormatFloat(effect.D))
        + ((effect.B == -1) ? " for unit " + FormatInt(effect.A) : " for class " + FormatInt(effect.B));
}

wxString AGE_Frame::GetEffectCmdName(int effect, int tech)
{
    wxString Name;
    switch(dataset->Effects[tech].EffectCommands[effect].Type)
    {
        case 30:
            Name = "Neutral ";
            goto AttributeModifierSet;
        case 20:
            Name = "Enemy ";
            goto AttributeModifierSet;
        case 10:
            Name = "Team ";
        case 0:
        AttributeModifierSet:
        {
            Name += "Set " + Tester(dataset->Effects[tech].EffectCommands[effect], " to ");
            break;
        }
        case 31:
            Name = "Neutral ";
            goto ResourceModifierSet;
        case 21:
            Name = "Enemy ";
            goto ResourceModifierSet;
        case 11:
            Name = "Team ";
        case 1:
        ResourceModifierSet:
            if(dataset->Effects[tech].EffectCommands[effect].B == 0)
            {
                Name += "Set resource "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                +" to "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
            }
            else
            {
                Name += "Change resource "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                +" by "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
            }
            if(GenieVersion >= genie::GV_C2 && GenieVersion <= genie::GV_LatestDE2)
            {
                if(dataset->Effects[tech].EffectCommands[effect].C >= 0)
                {
                    Name += " times resource "+FormatInt(dataset->Effects[tech].EffectCommands[effect].C);
                }
            }
            break;
        case 32:
            Name = "Neutral ";
            goto DisableEnable;
        case 22:
            Name = "Enemy ";
            goto DisableEnable;
        case 12:
            Name = "Team ";
        case 2:
        DisableEnable:
            if(dataset->Effects[tech].EffectCommands[effect].B == 0)
                Name += "Disable";
            else
                Name += "Enable";
            Name += " unit "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A);
            break;
        case 33:
            Name = "Neutral ";
            goto UpgradeUnit;
        case 23:
            Name = "Enemy ";
            goto UpgradeUnit;
        case 13:
            Name = "Team ";
        case 3:
        UpgradeUnit:
            Name += "Upgrade unit "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
            +" to "+FormatInt(dataset->Effects[tech].EffectCommands[effect].B);
            if(GenieVersion >= genie::GV_C2 && GenieVersion <= genie::GV_LatestDE2)
            {
                Name += dataset->Effects[tech].EffectCommands[effect].C == -1 ? ", all" : ", on map";
            }
            break;
        case 34:
            Name = "Neutral ";
            goto AttributeModifier;
        case 24:
            Name = "Enemy ";
            goto AttributeModifier;
        case 14:
            Name = "Team ";
        case 4:
        AttributeModifier:
        {
            Name += "Change " + Tester(dataset->Effects[tech].EffectCommands[effect], " by ");
            break;
        }
        case 35:
            Name = "Neutral ";
            goto AttributeModifierMultiply;
        case 25:
            Name = "Enemy ";
            goto AttributeModifierMultiply;
        case 15:
            Name = "Team ";
        case 5:
        AttributeModifierMultiply:
        {
            Name += "Multiply " + Tester(dataset->Effects[tech].EffectCommands[effect], " by ");
            break;
        }
        case 36:
            Name = "Neutral ";
            goto ResourceModifierMultiply;
        case 26:
            Name = "Enemy ";
            goto ResourceModifierMultiply;
        case 16:
            Name = "Team ";
        case 6:
        ResourceModifierMultiply:
            Name += "Multiply resource "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
            +" by "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
            break;
        case 37:
            Name = "Neutral ";
            goto SpawnUnit;
        case 27:
            Name = "Enemy ";
            goto SpawnUnit;
        case 17:
            Name = "Team ";
        case 7:
        SpawnUnit:
            if(GenieVersion >= genie::GV_C2 && GenieVersion <= genie::GV_LatestDE2)
            {
                Name = "Spawn unit "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                +" from "+FormatInt(dataset->Effects[tech].EffectCommands[effect].B)
                +", "+FormatInt(dataset->Effects[tech].EffectCommands[effect].C)+" times";
            }
            if(GameVersion == EV_UP)
            {
                if (dataset->Effects[tech].EffectCommands[effect].B == 1)
                {
                    Name += "Enable tech "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
                }
                else if (dataset->Effects[tech].EffectCommands[effect].B == 2)
                {
                    Name += "Force multi-use tech "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
                }
                else
                {
                    Name += "Disable tech "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
                }
            }
            break;
        case 38:
            Name = "Neutral ";
            goto ModifyTech;
        case 28:
            Name = "Enemy ";
            goto ModifyTech;
        case 18:
            Name = "Team ";
        case 8:
        ModifyTech:
            {
                if(dataset->Effects[tech].EffectCommands[effect].B == -2)
                {
                    Name += "Modify tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                    +" research time"
                    +" by "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
                }
                else if(dataset->Effects[tech].EffectCommands[effect].B >= 0 && dataset->Effects[tech].EffectCommands[effect].B <= 3)
                {
                    Name += "Modify tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                    +" cost type "+FormatInt(dataset->Effects[tech].EffectCommands[effect].B)
                    +" to "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
                }
                else if(dataset->Effects[tech].EffectCommands[effect].B >= 16384 && dataset->Effects[tech].EffectCommands[effect].B <= 16387)
                {
                    Name += "Modify tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                    +" cost type "+FormatInt(dataset->Effects[tech].EffectCommands[effect].B - 16384)
                    +" by "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
                }
                else
                {
                    Name += "Modify tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                    +" research time"
                    +" to "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
                }
            }
            break;
        case 39:
            Name = "Neutral ";
            goto SetPlayerCivName;
        case 29:
            Name = "Enemy ";
            goto SetPlayerCivName;
        case 19:
            Name = "Team ";
        case 9:
        SetPlayerCivName:
            Name += "Set player civ name to lang id "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D);
            break;
        case 101:
            //Name = "Tech Cost Modifier (Set/+/-)";
            if(dataset->Effects[tech].EffectCommands[effect].C == 0)
            {
                Name = "Set tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                +" cost type "+FormatInt(dataset->Effects[tech].EffectCommands[effect].B)
                +" to "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D)
                +" at gamestart";
            }
            else
            {
                Name = "Change tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                +" cost type "+FormatInt(dataset->Effects[tech].EffectCommands[effect].B)
                +" by "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D)
                +" at gamestart";
            }
            break;
        case 102:
            Name = "Disable tech "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D)
            +" at gamestart";
            break;
        case 103:
            //Name = "Tech Time Modifier (Set/+/-)";
            if(dataset->Effects[tech].EffectCommands[effect].C == 0)
            {
                Name = "Set tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                +" time to "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D)
                +" at gamestart";
            }
            else
            {
                Name = "Change tech "+FormatInt(dataset->Effects[tech].EffectCommands[effect].A)
                +" time by "+FormatFloat(dataset->Effects[tech].EffectCommands[effect].D)
                +" at gamestart";
            }
            break;
        default:
            Name = "No type/Invalid type";
    }
    return Name;
}

void AGE_Frame::OnEffectCmdSearch(wxCommandEvent &event)
{
    How2List = SEARCH;
    ListEffectCmds();
}

void AGE_Frame::ListEffectCmds()
{
    InitSearch(Techs_Effects_Search->GetValue().MakeLower(), Techs_Effects_Search_R->GetValue().MakeLower());
    SearchAnd = Techs_Effects_UseAnd[0]->GetValue();
    ExcludeAnd = Techs_Effects_UseAnd[1]->GetValue();

    Techs_Effects_ListV->Sweep();

    if(dataset->Effects.size())
    for(size_t loop = 0; loop < dataset->Effects[TechIDs.front()].EffectCommands.size(); ++loop)
    {
        wxString Name = FormatInt(loop)+" - "+GetEffectCmdName(loop, TechIDs.front());
        if(SearchMatches(" " + Name.Lower() + " "))
        {
            Techs_Effects_ListV->names.Add(Name);
            Techs_Effects_ListV->indexes.push_back(loop);
        }
    }
    RefreshList(Techs_Effects_ListV, &EffectIDs);

    SearchAnd = ExcludeAnd = false;

    wxCommandEvent e;
    OnEffectCmdSelect(e);
}

void AGE_Frame::OnEffectCmdSelect(wxCommandEvent &event)
{
    auto selections = Techs_Effects_ListV->GetSelectedCount();
    wxBusyCursor WaitCursor;
    for (auto &box : uiGroupTechEffect)
    {
        box->clear();
    }
    short attributeNameId = -1;
    short researchNameId = -1;
    bool enableD = true;
    if(selections > 0)
    {
        getSelectedItems(selections, Techs_Effects_ListV, EffectIDs);
        Effects_Type_Holder->Show(true);

        genie::EffectCommand * EffectPointer = nullptr;
        for(auto loop = selections; loop--> 0;)
        {
            EffectPointer = &dataset->Effects[TechIDs.front()].EffectCommands[EffectIDs[loop]];
            Effects_Type->prepend(&EffectPointer->Type);
            Effects_A->prepend(&EffectPointer->A);
            Effects_B->prepend(&EffectPointer->B);
            Effects_C->prepend(&EffectPointer->C);
            Effects_D->prepend(&EffectPointer->D);
        }

        if (EffectPointer->Type >= 0 && EffectPointer->Type <= 39)
        {
            Effects_Type_ComboBox->SetSelection(EffectPointer->Type + 1);
        }
        else if (EffectPointer->Type >= 101 && EffectPointer->Type <= 103)
        {
            Effects_Type_ComboBox->SetSelection(EffectPointer->Type - 60);
        }
        else
        {
            Effects_Type_ComboBox->SetSelection(0);
        }

        bool NeverHide = Effects_NeverHide->GetValue();
        switch(EffectPointer->Type)
        {
            case 20:
            case 30:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 0:
            case 10:
            {
                Effects_A_ComboBox->SwapList(&unit_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->TakeControl();
                Effects_B_ComboBox->SwapList(&class_names);
                Effects_B_ComboBox->Show(true);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->TakeControl();
                Effects_C_ComboBox->SwapList(&effect_attribute_names);
                Effects_C_ComboBox->Show(true);
                attributeNameId = EffectPointer->C;
                Effects_D_ComboBox->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(true);

                Effects_A_Text->SetLabel("Unit ");
                Effects_B_Text->SetLabel("Class ");
                Effects_C_Text->SetLabel("Attribute ");
                Effects_D_Text->SetLabel("Amount [Set] ");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");

                if(EffectPointer->C == 8 || EffectPointer->C == 9)
                {
                    enableD = NeverHide;
                    Effects_D->Show(false);
                    Effects_89_Amount->Show(true);
                    Effects_89_Amount->ChangeValue(lexical_cast<string>(uint16_t(EffectPointer->D) & 0xFF)); // Correct value
                    Effects_89_Type->Show(true);
                    Effects_89_Type_CB1->Show(true);
                    uint16_t attack_type = (uint16_t)EffectPointer->D >> 8;
                    Effects_89_Type->ChangeValue(lexical_cast<string>(attack_type)); // Correct class
                    Effects_89_Type_CB1->SetChoice(attack_type);
                    Effects_89_Type_Text->SetLabel("Type ");
                }
                else
                {
                    Effects_D->Show(true);
                    Effects_89_Amount->Show(false);
                    Effects_89_Type->Show(false);
                    Effects_89_Type_CB1->Show(false);
                    Effects_89_Type_Text->SetLabel("");
                }
            }
            break;
            case 21:
            case 31:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 1:
            case 11:
            {
                bool DE2 = GenieVersion >= genie::GV_C2 && GenieVersion <= genie::GV_LatestDE2;
                Effects_A_ComboBox->SwapList(&resource_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->TakeControl();
                Effects_B_CheckBox->Show(true);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->SwapList(&resource_names);
                Effects_C_ComboBox->Show(DE2);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(NeverHide || DE2);
                Effects_D->Show(true);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Resource ");
                Effects_B_Text->SetLabel("Mode ");
                Effects_C_Text->SetLabel("Resource [*] ");
                if(EffectPointer->B == 0)
                {
                    Effects_D_Text->SetLabel("Amount [Set] ");
                }
                else
                {
                    Effects_D_Text->SetLabel("Amount [+/-] ");
                }
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel(" [ ] = Set, [X] = +/-");
                Effects_Info_C->SetLabel(DE2 ? " [>=0] = Multiply with" : "");
            }
            break;
            case 22:
            case 32:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 2:
            case 12:
            {
                Effects_A_ComboBox->SwapList(&unit_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->TakeControl();
                Effects_B_CheckBox->Show(true);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(NeverHide);
                Effects_D->Show(NeverHide);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Unit ");
                Effects_B_Text->SetLabel("Mode ");
                Effects_C_Text->SetLabel("Unused ");
                Effects_D_Text->SetLabel("Unused ");
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel(" [ ] = Disable, [X] = Enable");
                Effects_Info_C->SetLabel("");
            }
            break;
            case 23:
            case 33:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 3:
            case 13:
            {
                bool DE2 = GenieVersion >= genie::GV_C2 && GenieVersion <= genie::GV_LatestDE2;
                Effects_A_ComboBox->SwapList(&unit_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->TakeControl();
                Effects_B_ComboBox->SwapList(&unit_names);
                Effects_B_ComboBox->Show(true);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(NeverHide || DE2);
                Effects_D->Show(NeverHide);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Unit ");
                Effects_B_Text->SetLabel("To Unit ");
                Effects_C_Text->SetLabel("Mode ");
                Effects_D_Text->SetLabel("Unused ");
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel(DE2 ? " [-1] = All, [!-1] = On map" : "");
            }
            break;
            case 24:
            case 34:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 4:
            case 14:
            {
                Effects_A_ComboBox->SwapList(&unit_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->TakeControl();
                Effects_B_ComboBox->SwapList(&class_names);
                Effects_B_ComboBox->Show(true);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->TakeControl();
                Effects_C_ComboBox->SwapList(&effect_attribute_names);
                Effects_C_ComboBox->Show(true);
                attributeNameId = EffectPointer->C;
                Effects_D_ComboBox->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(true);

                Effects_A_Text->SetLabel("Unit ");
                Effects_B_Text->SetLabel("Class ");
                Effects_C_Text->SetLabel("Attribute ");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");

                if(EffectPointer->C == 8 || EffectPointer->C == 9)
                {
                    enableD = NeverHide;
                    Effects_D->Show(false);
                    Effects_89_Amount->Show(true);
                    Effects_89_Amount->ChangeValue(lexical_cast<string>(uint16_t(EffectPointer->D) & 0xFF)); // Correct value
                    Effects_89_Type->Show(true);
                    Effects_89_Type_CB1->Show(true);
                    uint16_t attack_type = (uint16_t)EffectPointer->D >> 8;
                    Effects_89_Type->ChangeValue(lexical_cast<string>(attack_type)); // Correct class
                    Effects_89_Type_CB1->SetChoice(attack_type);
                    Effects_D_Text->SetLabel("Amount [+] ");
                    Effects_89_Type_Text->SetLabel("Type ");
                }
                else
                {
                    Effects_D->Show(true);
                    Effects_89_Amount->Show(false);
                    Effects_89_Type->Show(false);
                    Effects_89_Type_CB1->Show(false);
                    Effects_D_Text->SetLabel("Amount [+/-] ");
                    Effects_89_Type_Text->SetLabel("");
                }
            }
            break;
            case 25:
            case 35:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 5:
            case 15:
            {
                Effects_A_ComboBox->SwapList(&unit_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->TakeControl();
                Effects_B_ComboBox->SwapList(&class_names);
                Effects_B_ComboBox->Show(true);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->TakeControl();
                Effects_C_ComboBox->SwapList(&effect_attribute_names);
                Effects_C_ComboBox->Show(true);
                attributeNameId = EffectPointer->C;
                Effects_D_ComboBox->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(true);

                Effects_A_Text->SetLabel("Unit ");
                Effects_B_Text->SetLabel("Class ");
                Effects_C_Text->SetLabel("Attribute ");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");

                if(EffectPointer->C == 8 || EffectPointer->C == 9)
                {
                    enableD = NeverHide;
                    Effects_D->Show(false);
                    Effects_89_Amount->Show(true);
                    Effects_89_Amount->ChangeValue(lexical_cast<string>(uint16_t(EffectPointer->D) & 0xFF)); // Correct value
                    Effects_89_Type->Show(true);
                    Effects_89_Type_CB1->Show(true);
                    uint16_t attack_type = (uint16_t)EffectPointer->D >> 8;
                    Effects_89_Type->ChangeValue(lexical_cast<string>(attack_type)); // Correct class
                    Effects_89_Type_CB1->SetChoice(attack_type);
                    Effects_D_Text->SetLabel("Amount [%] ");
                    Effects_89_Type_Text->SetLabel("Type ");
                }
                else
                {
                    Effects_D->Show(true);
                    Effects_89_Amount->Show(false);
                    Effects_89_Type->Show(false);
                    Effects_89_Type_CB1->Show(false);
                    Effects_D_Text->SetLabel("Amount [*] ");
                    Effects_89_Type_Text->SetLabel("");
                }
            }
            break;
            case 26:
            case 36:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 6:
            case 16:
            {
                Effects_A_ComboBox->SwapList(&resource_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(NeverHide);
                Effects_C->Show(NeverHide);
                Effects_D->Show(true);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Resource ");
                Effects_B_Text->SetLabel("Unused ");
                Effects_C_Text->SetLabel("Unused ");
                Effects_D_Text->SetLabel("Amount [*] ");
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");
            }
            break;
            case 27:
            case 37:
            if(GenieVersion != genie::GV_TC)
            {
                goto noup;
            }
            case 7:
            case 17:
            if(GenieVersion >= genie::GV_C2 && GenieVersion <= genie::GV_LatestDE2)
            {
                Effects_A_ComboBox->SwapList(&unit_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->TakeControl();
                Effects_B_ComboBox->SwapList(&unit_names);
                Effects_B_ComboBox->Show(true);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(true);
                Effects_D->Show(NeverHide);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Unit ");
                Effects_B_Text->SetLabel("From Building ");
                Effects_C_Text->SetLabel("Amount ");
                Effects_D_Text->SetLabel("Unused ");
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");
            }
            if (GameVersion == EV_UP)
            {
                Effects_A_ComboBox->Show(false);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(true);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(NeverHide);
                Effects_B->Show(true);
                Effects_B->SetToolTip("0 Disable\n1 Enable\n2 Force enable");
                Effects_C->Show(NeverHide);
                Effects_D->Show(true);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Unused ");
                Effects_B_Text->SetLabel("Action * ");
                Effects_C_Text->SetLabel("Unused ");
                Effects_D_Text->SetLabel("Tech ");
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");
            }
            break;
            case 8:
            case 18:
            case 28:
            case 38:
            {
                Effects_A_ComboBox->SwapList(&research_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->TakeControl();
                Effects_B_ComboBox->SwapList(&modify_research_names);
                Effects_B_ComboBox->Show(true);
                researchNameId = EffectPointer->B;
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_B->SetToolTip("0-3 and 16384-16387 only work if the tech has the corresponding cost/storage set");
                Effects_C->Show(false);

                Effects_A_Text->SetLabel("Tech ");
                Effects_B_Text->SetLabel("Action * ");
                Effects_C_Text->SetLabel("Unused ");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");

                Effects_D->Show(true);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_D_Text->SetLabel("Amount ");
                Effects_89_Type_Text->SetLabel("");
            }
            break;
            case 9:
            case 19:
            case 29:
            case 39:
            {
                Effects_A_ComboBox->Show(false);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_A->SetToolTip("Needs to be 0, do not touch");
                Effects_B->Show(true);
                Effects_B->SetToolTip("Needs to be 0, do not touch");
                Effects_C->Show(NeverHide);
                Effects_D->Show(true);
                Effects_D->SetToolTip("An ID from the language file");
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Data * ");
                Effects_B_Text->SetLabel("Action * ");
                Effects_C_Text->SetLabel("Unused ");
                Effects_D_Text->SetLabel("Lang ID * ");
                Effects_89_Type_Text->SetLabel("");

                Effects_A->SetValue("0");
                Effects_A->SaveEdits();
                Effects_B->SetValue("0");
                Effects_B->SaveEdits();

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");
            }
            break;
            case 101:
            {
                Effects_A_ComboBox->SwapList(&research_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->TakeControl();
                Effects_B_ComboBox->SwapList(&resource_names);
                Effects_B_ComboBox->Show(true);
                Effects_C_CheckBox->TakeControl();
                Effects_C_CheckBox->Show(true);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(true);
                Effects_C->Show(true);
                Effects_D->Show(true);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Tech ");
                Effects_B_Text->SetLabel("Resource ");
                Effects_C_Text->SetLabel("Mode ");
                if(EffectPointer->C == 0)
                {
                    Effects_D_Text->SetLabel("Amount [Set] ");
                }
                else
                {
                    Effects_D_Text->SetLabel("Amount [+/-] ");
                }
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel(" [ ] = Set, [X] = +/-");
            }
            break;
            case 102:
            {
                Effects_A_ComboBox->Show(false);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(true);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(NeverHide);
                Effects_B->Show(NeverHide);
                Effects_C->Show(NeverHide);
                Effects_D->Show(true);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Unused ");
                Effects_B_Text->SetLabel("Unused ");
                Effects_C_Text->SetLabel("Unused ");
                Effects_D_Text->SetLabel("Tech ");
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");
            }
            break;
            case 103:
            {
                Effects_A_ComboBox->SwapList(&research_names);
                Effects_A_ComboBox->Show(true);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->TakeControl();
                Effects_C_CheckBox->Show(true);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(true);
                Effects_B->Show(NeverHide);
                Effects_C->Show(true);
                Effects_D->Show(true);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Tech ");
                Effects_B_Text->SetLabel("Unused ");
                Effects_C_Text->SetLabel("Mode ");
                if(EffectPointer->C == 0)
                {
                    Effects_D_Text->SetLabel("Amount [Set] ");
                }
                else
                {
                    Effects_D_Text->SetLabel("Amount [+/-] ");
                }
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel(" [ ] = Set, [X] = +/-");
            }
            break;
            noup:
            default:
            {
                Effects_A_ComboBox->Show(false);
                Effects_B_CheckBox->Show(false);
                Effects_B_ComboBox->Show(false);
                Effects_C_CheckBox->Show(false);
                Effects_C_ComboBox->Show(false);
                Effects_D_ComboBox->Show(false);
                Effects_89_Type_CB1->Show(false);
                Effects_A->Show(NeverHide);
                Effects_B->Show(NeverHide);
                Effects_C->Show(NeverHide);
                Effects_D->Show(NeverHide);
                Effects_89_Amount->Show(false);
                Effects_89_Type->Show(false);

                Effects_A_Text->SetLabel("Attribute A ");
                Effects_B_Text->SetLabel("Attribute B ");
                Effects_C_Text->SetLabel("Attribute C ");
                Effects_D_Text->SetLabel("Attribute D ");
                Effects_89_Type_Text->SetLabel("");

                Effects_Info_B->SetLabel("");
                Effects_Info_C->SetLabel("");
            }
        }
    }
    else
    {
        Effects_Type_Holder->Show(false);
        Effects_A_ComboBox->Show(false);
        Effects_B_CheckBox->Show(false);
        Effects_B_ComboBox->Show(false);
        Effects_C_CheckBox->Show(false);
        Effects_C_ComboBox->Show(false);
        Effects_D_ComboBox->Show(false);
        Effects_89_Type_CB1->Show(false);
        Effects_A->Show(false);
        Effects_B->Show(false);
        Effects_C->Show(false);
        Effects_D->Show(false);
        Effects_89_Amount->Show(false);
        Effects_89_Type->Show(false);
        Effects_A_Text->SetLabel("Select effect first ");
        Effects_B_Text->SetLabel("");
        Effects_C_Text->SetLabel("");
        Effects_D_Text->SetLabel("");
        Effects_89_Type_Text->SetLabel("");
        Effects_Info_B->SetLabel("");
        Effects_Info_C->SetLabel("");
    }
    for (auto &box : uiGroupTechEffect)
    {
        box->update();
    }
    Effects_D->Enable(enableD);
    Effects_Type_Holder->Layout();
    Effects_Data_Holder->Layout();
    if (attributeNameId != -1)
    {
        if (attributeNameId >= 0 && attributeNameId <= 23)
        {
            Effects_C_ComboBox->SetSelection(attributeNameId + 1);
        }
        else if (attributeNameId >= 100 && attributeNameId <= 109)
        {
            Effects_C_ComboBox->SetSelection(attributeNameId - 75);
        }
        else
        {
            Effects_C_ComboBox->SetSelection(0);
        }
    }
    if (researchNameId != -1)
    {
        if (researchNameId == -2)
        {
            Effects_B_ComboBox->SetSelection(1);
        }
        else if (researchNameId >= 0 && researchNameId <= 4)
        {
            Effects_B_ComboBox->SetSelection(researchNameId + 2);
        }
        else if (researchNameId >= 16384 && researchNameId <= 16387)
        {
            Effects_B_ComboBox->SetSelection(researchNameId - 16378);
        }
        else
        {
            Effects_B_ComboBox->SetSelection(0);
        }
    }
//  Refresh(); // Too much lag.
}

void AGE_Frame::OnEffectCmdAdd(wxCommandEvent &event) // Works.
{
    auto selections = Techs_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    AddToList(dataset->Effects[TechIDs.front()].EffectCommands);
    ListEffectCmds();
}

void AGE_Frame::OnEffectCmdInsert(wxCommandEvent &event)  // Works.
{
    auto selections = Techs_Effects_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    InsertToList(dataset->Effects[TechIDs.front()].EffectCommands, EffectIDs.front());
    ListEffectCmds();
}

void AGE_Frame::OnEffectCmdDelete(wxCommandEvent &event)  // Works.
{
    auto selections = Techs_Effects_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    DeleteFromList(dataset->Effects[TechIDs.front()].EffectCommands, EffectIDs);
    ListEffectCmds();
}

void AGE_Frame::OnEffectCmdCopy(wxCommandEvent &event)    // Works.
{
    auto selections = Techs_Effects_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    CopyFromList(dataset->Effects[TechIDs.front()].EffectCommands, EffectIDs, copies.EffectCmd);
    Techs_Effects_ListV->SetFocus();
}

void AGE_Frame::OnEffectCmdPaste(wxCommandEvent &event)   // Works.
{
    auto selections = Techs_Effects_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    PasteToList(dataset->Effects[TechIDs.front()].EffectCommands, EffectIDs, copies.EffectCmd);
    ListEffectCmds();
}

void AGE_Frame::OnEffectCmdPasteInsert(wxCommandEvent &event) // Works.
{
    auto selections = Techs_Effects_ListV->GetSelectedCount();
    if(selections < 1) return;

    wxBusyCursor WaitCursor;
    PasteInsertToList(dataset->Effects[TechIDs.front()].EffectCommands, EffectIDs.front(), copies.EffectCmd);
    ListEffectCmds();
}

void AGE_Frame::OnEffectCmdCopyToTechs(wxCommandEvent &event)
{
    for(size_t loop=1; loop < TechIDs.size(); ++loop)
    {
        dataset->Effects[TechIDs[loop]].EffectCommands = dataset->Effects[TechIDs.front()].EffectCommands;
    }
}

void AGE_Frame::LoadAllEffects(wxCommandEvent &event)
{
    InitSearch(Techs_AllEffects_Search->GetValue().MakeLower(), Techs_AllEffects_Search_R->GetValue().MakeLower());
    SearchAnd = Techs_AllEffects_UseAnd[0]->GetValue();
    ExcludeAnd = Techs_AllEffects_UseAnd[1]->GetValue();

    Techs_AllEffects_ListV->Sweep();

    for(short tech = 0; tech < dataset->Effects.size(); ++tech)
    {
        for(short effect = 0; effect < dataset->Effects[tech].EffectCommands.size(); ++effect)
        {
            wxString Name = " T"+lexical_cast<string>(tech)+" E"+lexical_cast<string>(effect)+" - "+GetEffectCmdName(effect, tech);
            if(SearchMatches(" " + Name.Lower() + " "))
            {
                Techs_AllEffects_ListV->names.Add(Name);
            }
        }
    }

    RefreshList(Techs_AllEffects_ListV);

    SearchAnd = ExcludeAnd = false;

    wxCommandEvent e;
    OnAllEffectSelect(e);
}

void AGE_Frame::ClearAllEffects(wxCommandEvent &event)
{
    Techs_Search->SetValue("");
    Techs_Effects_Search->SetValue("");
}

void AGE_Frame::OnAllEffectSelect(wxCommandEvent &event)
{
    SearchAllSubVectors(Techs_AllEffects_ListV, Techs_Search, Techs_Effects_Search);
}

void AGE_Frame::CreateTechControls()
{
    Tab_Techs = new ATabPage(TabBar_Main);

    Techs_Main = new wxBoxSizer(wxHORIZONTAL);
    Techs_Buttons = new wxGridSizer(3, 0, 0);
    Effects_DataArea = new wxBoxSizer(wxVERTICAL);
    Effects_ListArea = new wxBoxSizer(wxVERTICAL);
    Techs_Effects_Buttons = new wxGridSizer(3, 0, 0);
    Effects_DataAreaProperties = new wxBoxSizer(wxVERTICAL);
    Techs_Techs = new wxStaticBoxSizer(wxVERTICAL, Tab_Techs, "Effects");
    Techs_Searches[0] = new wxBoxSizer(wxHORIZONTAL);
    Techs_Searches[1] = new wxBoxSizer(wxHORIZONTAL);
    Techs_Search = new wxTextCtrl(Tab_Techs, wxID_ANY);
    Techs_UseAnd[0] = new wxCheckBox(Tab_Techs, wxID_ANY, "And");
    Techs_Search_R = new wxTextCtrl(Tab_Techs, wxID_ANY);
    Techs_UseAnd[1] = new wxCheckBox(Tab_Techs, wxID_ANY, "And");
    Techs_ListV = new ProperList(Tab_Techs, wxSize(200, 100));
    Techs_Add = new wxButton(Tab_Techs, wxID_ANY, "Add", wxDefaultPosition, wxSize(10, -1));
    Techs_Insert = new wxButton(Tab_Techs, wxID_ANY, "Insert New", wxDefaultPosition, wxSize(10, -1));
    Techs_Delete = new wxButton(Tab_Techs, wxID_ANY, "Delete", wxDefaultPosition, wxSize(10, -1));
    Techs_Copy = new wxButton(Tab_Techs, wxID_ANY, "Copy", wxDefaultPosition, wxSize(10, -1));
    Techs_Paste = new wxButton(Tab_Techs, wxID_ANY, "Paste", wxDefaultPosition, wxSize(10, -1));
    Techs_PasteInsert = new wxButton(Tab_Techs, wxID_ANY, "Ins Copies", wxDefaultPosition, wxSize(10, -1));
    Techs_Rename = new wxButton(Tab_Techs, wxID_ANY, "Rename effects");
    Techs_Restore = new wxButton(Tab_Techs, wxID_ANY, "Rename for GeniEd 2");

    Techs_Name_Text = new SolidText(Tab_Techs, " Effect Name");
    Techs_Name = new StringControl(Tab_Techs, this, nullptr, 31, false);

    Techs_Effects = new wxStaticBoxSizer(wxVERTICAL, Tab_Techs, "Effect Commands");
    Techs_Effects_Searches[0] = new wxBoxSizer(wxHORIZONTAL);
    Techs_Effects_Searches[1] = new wxBoxSizer(wxHORIZONTAL);
    Techs_Effects_Search = new wxTextCtrl(Tab_Techs, wxID_ANY);
    Techs_Effects_UseAnd[0] = new wxCheckBox(Tab_Techs, wxID_ANY, "And");
    Techs_Effects_Search_R = new wxTextCtrl(Tab_Techs, wxID_ANY);
    Techs_Effects_UseAnd[1] = new wxCheckBox(Tab_Techs, wxID_ANY, "And");
    Techs_Effects_ListV = new ProperList(Tab_Techs, wxSize(200, 100));
    Techs_Effects_Add = new wxButton(Tab_Techs, wxID_ANY, "Add", wxDefaultPosition, wxSize(10, -1));
    Techs_Effects_Insert = new wxButton(Tab_Techs, wxID_ANY, "Insert New", wxDefaultPosition, wxSize(10, -1));
    Techs_Effects_Delete = new wxButton(Tab_Techs, wxID_ANY, "Delete", wxDefaultPosition, wxSize(10, -1));
    Techs_Effects_Copy = new wxButton(Tab_Techs, wxID_ANY, "Copy", wxDefaultPosition, wxSize(10, -1));
    Techs_Effects_Paste = new wxButton(Tab_Techs, wxID_ANY, "Paste", wxDefaultPosition, wxSize(10, -1));
    Techs_Effects_PasteInsert = new wxButton(Tab_Techs, wxID_ANY, "Ins Copies", wxDefaultPosition, wxSize(10, -1));
    Techs_Effects_CopyToTechs = new wxButton(Tab_Techs, wxID_ANY, "Copy all to selected effects", wxDefaultPosition, wxSize(10, -1));

    Effects_Type_Holder = new wxBoxSizer(wxVERTICAL);
    Effects_Type2_Holder = new wxBoxSizer(wxHORIZONTAL);
    Effects_Type_Text = new SolidText(Tab_Techs, " Command Type *");
    Effects_Type = new NumberControl(CUByte, Tab_Techs, this, &uiGroupTechEffect, false);
    Effects_Type->SetToolTip("101 and 103 are only for\ntech tree and team bonus"
        "\n101-103 only work at gamestart\n7-39 are only available with UserPatch 1.5"
        "\n10-16 are also available with AoK HD");
    Effects_Type_ComboBox = new AGEComboBox(Tab_Techs, &effect_type_names, AGETextCtrl::GIANT);
    Effects_Data_Holder = new wxStaticBoxSizer(wxVERTICAL, Tab_Techs, "Effect Attributes");
    Effects_NeverHide = new wxCheckBox(Tab_Techs, wxID_ANY, "Never hide attributes", wxDefaultPosition, wxDefaultSize);
    Effects_NeverHide->SetValue(NeverHideAttributes);
    Effects_DataA_Holder = new wxBoxSizer(wxHORIZONTAL);
    Effects_DataB_Holder = new wxBoxSizer(wxHORIZONTAL);
    Effects_DataC_Holder = new wxBoxSizer(wxHORIZONTAL);
    Effects_DataD_Holder = new wxBoxSizer(wxHORIZONTAL);
    Effects_DataF_Holder = new wxBoxSizer(wxHORIZONTAL);
    Effects_A_Holder = new wxBoxSizer(wxVERTICAL);
    Effects_B_Holder = new wxBoxSizer(wxVERTICAL);
    Effects_C_Holder = new wxBoxSizer(wxVERTICAL);
    Effects_D_Holder = new wxBoxSizer(wxHORIZONTAL);
    Effects_89_Type_Holder = new wxBoxSizer(wxVERTICAL);
    Effects_A_Text = new SolidText(Tab_Techs, "Attribute A ", wxALIGN_RIGHT | wxST_NO_AUTORESIZE, wxSize(100, -1));
    Effects_A = new NumberControl(CShort, Tab_Techs, this, &uiGroupTechEffect, false, AGETextCtrl::LARGE);
    Effects_A_ComboBox = new LinkedComboBox(Tab_Techs, Effects_A, &unit_names, false);
    UnitComboBoxList.push_back(Effects_A_ComboBox);
    ResourceComboBoxList.push_back(Effects_A_ComboBox);
    ResearchComboBoxList.push_back(Effects_A_ComboBox);
    Effects_Info_A = new SolidText(Tab_Techs, "");
    Effects_B_Text = new SolidText(Tab_Techs, "Attribute B ", wxALIGN_RIGHT | wxST_NO_AUTORESIZE, wxSize(100, -1));
    Effects_B = new NumberControl(CShort, Tab_Techs, this, &uiGroupTechEffect, false, AGETextCtrl::LARGE);
    Effects_B_Boxes = new wxBoxSizer(wxHORIZONTAL);
    Effects_B_CheckBox = new LinkedCheckBox(Tab_Techs, "", Effects_B, false);
    Effects_B_ComboBox = new LinkedComboBox(Tab_Techs, Effects_B, &class_names, false);
    UnitComboBoxList.push_back(Effects_B_ComboBox);
    ResourceComboBoxList.push_back(Effects_B_ComboBox);
    Effects_Info_B = new SolidText(Tab_Techs, " Info B");
    Effects_C_Text = new SolidText(Tab_Techs, "Attribute C ", wxALIGN_RIGHT | wxST_NO_AUTORESIZE, wxSize(100, -1));
    Effects_C = new NumberControl(CShort, Tab_Techs, this, &uiGroupTechEffect, false, AGETextCtrl::LARGE);
    Effects_C_Boxes = new wxBoxSizer(wxHORIZONTAL);
    Effects_C_CheckBox = new LinkedCheckBox(Tab_Techs, "", Effects_C, false);
    Effects_C_ComboBox = new LinkedComboBox(Tab_Techs, Effects_C, &effect_attribute_names, false);
    Effects_Info_C = new SolidText(Tab_Techs, " Info C");
    Effects_D_Text = new SolidText(Tab_Techs, "Attribute D ", wxALIGN_RIGHT | wxST_NO_AUTORESIZE, wxSize(100, -1));
    Effects_D = new NumberControl(CFloat, Tab_Techs, this, &uiGroupTechEffect, false, AGETextCtrl::LARGE);
    Effects_D_Boxes = new wxBoxSizer(wxHORIZONTAL);
    Effects_D_ComboBox = new LinkedComboBox(Tab_Techs, Effects_D, &research_names, false);
    ResearchComboBoxList.push_back(Effects_D_ComboBox);
    Effects_Info_D = new SolidText(Tab_Techs, "");
    Effects_89_Amount = new NumberControl(CUByte, Tab_Techs, this, nullptr, false, AGETextCtrl::LARGE);
    Effects_89_Type_Text = new SolidText(Tab_Techs, "Type ", wxALIGN_RIGHT | wxST_NO_AUTORESIZE, wxSize(100, -1));
    Effects_89_Type = new NumberControl(CUByte, Tab_Techs, this, nullptr, false, AGETextCtrl::LARGE);
    Effects_89_Type_CB1 = new LinkedComboBox(Tab_Techs, Effects_89_Type, &armor_names, false);

    Techs_AllEffects = new wxStaticBoxSizer(wxVERTICAL, Tab_Techs, "Commands of all Effects");
    Techs_AllEffects_Searches[0] = new wxBoxSizer(wxHORIZONTAL);
    Techs_AllEffects_Searches[1] = new wxBoxSizer(wxHORIZONTAL);
    Techs_AllEffects_Search = new wxTextCtrl(Tab_Techs, wxID_ANY);
    Techs_AllEffects_UseAnd[0] = new wxCheckBox(Tab_Techs, wxID_ANY, "And");
    Techs_AllEffects_Search_R = new wxTextCtrl(Tab_Techs, wxID_ANY);
    Techs_AllEffects_UseAnd[1] = new wxCheckBox(Tab_Techs, wxID_ANY, "And");
    Techs_AllEffects_ListV = new ProperList(Tab_Techs, wxSize(200, 100));
    Techs_AllEffects_Buttons = new wxBoxSizer(wxHORIZONTAL);
    Techs_AllEffects_Load = new wxButton(Tab_Techs, wxID_ANY, "Reload", wxDefaultPosition, wxSize(10, -1));
    Techs_AllEffects_Clear = new wxButton(Tab_Techs, wxID_ANY, "Clear *", wxDefaultPosition, wxSize(10, -1));
    Techs_AllEffects_Clear->SetToolTip("Clear the modified search texts");

    Techs_Buttons->Add(Techs_Add, 1, wxEXPAND);
    Techs_Buttons->Add(Techs_Delete, 1, wxEXPAND);
    Techs_Buttons->Add(Techs_Insert, 1, wxEXPAND);
    Techs_Buttons->Add(Techs_Copy, 1, wxEXPAND);
    Techs_Buttons->Add(Techs_Paste, 1, wxEXPAND);
    Techs_Buttons->Add(Techs_PasteInsert, 1, wxEXPAND);

    Techs_Searches[0]->Add(Techs_Search, 1, wxEXPAND);
    Techs_Searches[0]->Add(Techs_UseAnd[0], 0, wxLEFT, 2);
    Techs_Searches[1]->Add(Techs_Search_R, 1, wxEXPAND);
    Techs_Searches[1]->Add(Techs_UseAnd[1], 0, wxLEFT, 2);
    Techs_Techs->Add(Techs_Searches[0], 0, wxEXPAND);
    Techs_Techs->Add(Techs_Searches[1], 0, wxEXPAND);
    Techs_Techs->Add(Techs_ListV, 1, wxEXPAND | wxBOTTOM | wxTOP, 2);
    Techs_Techs->Add(Techs_Buttons, 0, wxEXPAND);

    Techs_Effects_Buttons->Add(Techs_Effects_Add, 1, wxEXPAND);
    Techs_Effects_Buttons->Add(Techs_Effects_Delete, 1, wxEXPAND);
    Techs_Effects_Buttons->Add(Techs_Effects_Insert, 1, wxEXPAND);
    Techs_Effects_Buttons->Add(Techs_Effects_Copy, 1, wxEXPAND);
    Techs_Effects_Buttons->Add(Techs_Effects_Paste, 1, wxEXPAND);
    Techs_Effects_Buttons->Add(Techs_Effects_PasteInsert, 1, wxEXPAND);

    Effects_ListArea->Add(Techs_Name_Text);
    Effects_ListArea->Add(Techs_Name, 0, wxEXPAND);
    Effects_ListArea->Add(Techs_Rename, 0, wxEXPAND);
    Effects_ListArea->Add(Techs_Restore, 0, wxEXPAND);

    Techs_Effects_Searches[0]->Add(Techs_Effects_Search, 1, wxEXPAND);
    Techs_Effects_Searches[0]->Add(Techs_Effects_UseAnd[0], 0, wxLEFT, 2);
    Techs_Effects_Searches[1]->Add(Techs_Effects_Search_R, 1, wxEXPAND);
    Techs_Effects_Searches[1]->Add(Techs_Effects_UseAnd[1], 0, wxLEFT, 2);
    Techs_Effects->Add(Techs_Effects_Searches[0], 0, wxEXPAND);
    Techs_Effects->Add(Techs_Effects_Searches[1], 0, wxEXPAND);
    Techs_Effects->Add(Techs_Effects_ListV, 1, wxEXPAND | wxBOTTOM | wxTOP, 2);
    Techs_Effects->Add(Techs_Effects_Buttons, 0, wxEXPAND);
    Techs_Effects->Add(Techs_Effects_CopyToTechs, 0, wxEXPAND | wxTOP, 2);

    Effects_ListArea->Add(Techs_Effects, 1, wxEXPAND | wxTOP, 5);

    Effects_Type2_Holder->Add(Effects_Type, 0, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_Type2_Holder->Add(Effects_Type_ComboBox, 0, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_Type_Holder->Add(Effects_Type_Text, 0, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_Type_Holder->Add(Effects_Type2_Holder);

    Effects_A_Holder->Add(Effects_A, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_A_Holder->Add(Effects_A_ComboBox, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);

    Effects_DataA_Holder->Add(Effects_A_Text);
    Effects_DataA_Holder->Add(Effects_A_Holder);
    Effects_DataA_Holder->Add(Effects_Info_A);

    Effects_B_Boxes->Add(Effects_B_CheckBox);
    Effects_B_Boxes->Add(Effects_B_ComboBox);
    Effects_B_Holder->Add(Effects_B, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_B_Holder->Add(Effects_B_Boxes, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);

    Effects_DataB_Holder->Add(Effects_B_Text);
    Effects_DataB_Holder->Add(Effects_B_Holder);
    Effects_DataB_Holder->Add(Effects_Info_B);

    Effects_C_Boxes->Add(Effects_C_CheckBox);
    Effects_C_Boxes->Add(Effects_C_ComboBox);
    Effects_C_Holder->Add(Effects_C, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_C_Holder->Add(Effects_C_Boxes, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);

    Effects_DataC_Holder->Add(Effects_C_Text);
    Effects_DataC_Holder->Add(Effects_C_Holder);
    Effects_DataC_Holder->Add(Effects_Info_C);

    Effects_D_Boxes->Add(Effects_D_ComboBox);
    Effects_D_Holder->Add(Effects_D);
    Effects_D_Holder->Add(Effects_89_Amount);

    Effects_DataD_Holder->Add(Effects_D_Text);
    Effects_DataD_Holder->Add(Effects_D_Holder, 0, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_DataD_Holder->Add(Effects_Info_D);

    Effects_89_Type_Holder->Add(Effects_89_Type, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_89_Type_Holder->Add(Effects_89_Type_CB1, 1, wxRESERVE_SPACE_EVEN_IF_HIDDEN);
    Effects_D_Boxes->Add(Effects_89_Type_Holder);

    Effects_DataF_Holder->Add(Effects_89_Type_Text);
    Effects_DataF_Holder->Add(Effects_D_Boxes);

    Effects_Data_Holder->Add(Effects_NeverHide);
    Effects_Data_Holder->Add(Effects_DataA_Holder, 0, wxTOP, 5);
    Effects_Data_Holder->Add(Effects_DataB_Holder, 0, wxTOP, 5);
    Effects_Data_Holder->Add(Effects_DataC_Holder, 0, wxTOP, 5);
    Effects_Data_Holder->Add(Effects_DataD_Holder, 0, wxTOP, 5);
    Effects_Data_Holder->Add(Effects_DataF_Holder);

    Techs_AllEffects_Searches[0]->Add(Techs_AllEffects_Search, 1, wxEXPAND);
    Techs_AllEffects_Searches[0]->Add(Techs_AllEffects_UseAnd[0], 0, wxLEFT, 2);
    Techs_AllEffects_Searches[1]->Add(Techs_AllEffects_Search_R, 1, wxEXPAND);
    Techs_AllEffects_Searches[1]->Add(Techs_AllEffects_UseAnd[1], 0, wxLEFT, 2);
    Techs_AllEffects->Add(Techs_AllEffects_Searches[0], 0, wxEXPAND);
    Techs_AllEffects->Add(Techs_AllEffects_Searches[1], 0, wxEXPAND);
    Techs_AllEffects->Add(Techs_AllEffects_ListV, 1, wxEXPAND | wxBOTTOM | wxTOP, 2);
    Techs_AllEffects_Buttons->Add(Techs_AllEffects_Load, 2, wxEXPAND | wxRIGHT, 2);
    Techs_AllEffects_Buttons->Add(Techs_AllEffects_Clear, 1, wxEXPAND);
    Techs_AllEffects->Add(Techs_AllEffects_Buttons, 0, wxEXPAND);

    Effects_DataArea->Add(Effects_Type_Holder, 0, wxEXPAND);
    Effects_DataArea->Add(Effects_Data_Holder, 0, wxEXPAND | wxTOP, 5);
    Effects_DataArea->Add(Techs_AllEffects, 1, wxEXPAND | wxTOP, 5);

    Techs_Main->Add(Techs_Techs, 21, wxEXPAND | wxALL, 5); // 3
    Techs_Main->Add(Effects_ListArea, 29, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 5); // 3+1
    Techs_Main->Add(Effects_DataArea, 31, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 5); // 6-1

    Effects_89_Amount->Show(false);
    Effects_89_Type->Show(false);
    Effects_A_ComboBox->Show(false);
    Effects_B_CheckBox->Show(false);
    Effects_B_ComboBox->Show(false);
    Effects_C_CheckBox->Show(false);
    Effects_C_ComboBox->Show(false);
    Effects_D_ComboBox->Show(false);
    Effects_89_Type_CB1->Show(false);

    Tab_Techs->SetSizer(Techs_Main);

    for(size_t loop = 0; loop < 2; ++loop)
    {
        Techs_UseAnd[loop]->Bind(wxEVT_CHECKBOX, &AGE_Frame::OnEffectSearch, this);
        Techs_Effects_UseAnd[loop]->Bind(wxEVT_CHECKBOX, &AGE_Frame::OnEffectCmdSearch, this);
        Techs_AllEffects_UseAnd[loop]->Bind(wxEVT_CHECKBOX, &AGE_Frame::LoadAllEffects, this);
    }
    Techs_Rename->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectRename, this);
    Techs_Restore->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectRenameGE2, this);
    Techs_ListV->Bind(wxEVT_LISTBOX, &AGE_Frame::OnEffectSelect, this);
    Techs_Search->Bind(wxEVT_TEXT, &AGE_Frame::OnEffectSearch, this);
    Techs_Search_R->Bind(wxEVT_TEXT, &AGE_Frame::OnEffectSearch, this);
    Techs_Effects_ListV->Bind(wxEVT_LISTBOX, &AGE_Frame::OnEffectCmdSelect, this);
    Techs_Effects_Search->Bind(wxEVT_TEXT, &AGE_Frame::OnEffectCmdSearch, this);
    Techs_Effects_Search_R->Bind(wxEVT_TEXT, &AGE_Frame::OnEffectCmdSearch, this);
    Techs_Add->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectAdd, this);
    Techs_Insert->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectInsert, this);
    Techs_Delete->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectDelete, this);
    Techs_Copy->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCopy, this);
    Techs_Paste->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectPaste, this);
    Techs_PasteInsert->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectPasteInsert, this);
    Techs_Effects_Add->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCmdAdd, this);
    Techs_Effects_Insert->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCmdInsert, this);
    Techs_Effects_Delete->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCmdDelete, this);
    Techs_Effects_Copy->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCmdCopy, this);
    Techs_Effects_Paste->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCmdPaste, this);
    Techs_Effects_PasteInsert->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCmdPasteInsert, this);
    Techs_Effects_CopyToTechs->Bind(wxEVT_BUTTON, &AGE_Frame::OnEffectCmdCopyToTechs, this);
    Effects_89_Amount->Bind(wxEVT_KILL_FOCUS, &AGE_Frame::OnKillFocus_Effects89, this);
    Effects_89_Amount->Bind(wxEVT_TEXT_ENTER, &AGE_Frame::OnEnter_Effects89, this);
    Effects_89_Type->Bind(wxEVT_KILL_FOCUS, &AGE_Frame::OnKillFocus_Effects89, this);
    Effects_89_Type->Bind(wxEVT_TEXT_ENTER, &AGE_Frame::OnEnter_Effects89, this);
    Techs_AllEffects_Search->Bind(wxEVT_TEXT, &AGE_Frame::LoadAllEffects, this);
    Techs_AllEffects_Search_R->Bind(wxEVT_TEXT, &AGE_Frame::LoadAllEffects, this);
    Techs_AllEffects_ListV->Bind(wxEVT_LISTBOX, &AGE_Frame::OnAllEffectSelect, this);
    Techs_AllEffects_Load->Bind(wxEVT_BUTTON, &AGE_Frame::LoadAllEffects, this);
    Techs_AllEffects_Clear->Bind(wxEVT_BUTTON, &AGE_Frame::ClearAllEffects, this);

    Techs_Name->Bind(wxEVT_KILL_FOCUS, [this](wxFocusEvent& event)
    {
        event.Skip();
        if (static_cast<AGETextCtrl*>(event.GetEventObject())->SaveEdits() == 0)
        {
            ListEffects();
        }
    });
    Techs_Name->Bind(wxEVT_TEXT_ENTER, [this](wxCommandEvent& event)
    {
        static_cast<AGETextCtrl*>(event.GetEventObject())->SaveEdits(true);
        ListEffects();
    });
    Effects_Type->Bind(wxEVT_KILL_FOCUS, &AGE_Frame::OnKillFocus_Techs, this);
    Effects_Type->Bind(wxEVT_TEXT_ENTER, &AGE_Frame::OnEnter_Techs, this);
    Effects_Type_ComboBox->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
    {
        switch (event.GetSelection())
        {
            case 1: Effects_Type->ChangeValue("0"); break;
            case 2: Effects_Type->ChangeValue("1"); break;
            case 3: Effects_Type->ChangeValue("2"); break;
            case 4: Effects_Type->ChangeValue("3"); break;
            case 5: Effects_Type->ChangeValue("4"); break;
            case 6: Effects_Type->ChangeValue("5"); break;
            case 7: Effects_Type->ChangeValue("6"); break;
            case 8: Effects_Type->ChangeValue("7"); break;
            case 9: Effects_Type->ChangeValue("8"); break;
            case 10: Effects_Type->ChangeValue("9"); break;
            case 11: Effects_Type->ChangeValue("10"); break;
            case 12: Effects_Type->ChangeValue("11"); break;
            case 13: Effects_Type->ChangeValue("12"); break;
            case 14: Effects_Type->ChangeValue("13"); break;
            case 15: Effects_Type->ChangeValue("14"); break;
            case 16: Effects_Type->ChangeValue("15"); break;
            case 17: Effects_Type->ChangeValue("16"); break;
            case 18: Effects_Type->ChangeValue("17"); break;
            case 19: Effects_Type->ChangeValue("18"); break;
            case 20: Effects_Type->ChangeValue("19"); break;
            case 21: Effects_Type->ChangeValue("20"); break;
            case 22: Effects_Type->ChangeValue("21"); break;
            case 23: Effects_Type->ChangeValue("22"); break;
            case 24: Effects_Type->ChangeValue("23"); break;
            case 25: Effects_Type->ChangeValue("24"); break;
            case 26: Effects_Type->ChangeValue("25"); break;
            case 27: Effects_Type->ChangeValue("26"); break;
            case 28: Effects_Type->ChangeValue("27"); break;
            case 29: Effects_Type->ChangeValue("28"); break;
            case 30: Effects_Type->ChangeValue("29"); break;
            case 31: Effects_Type->ChangeValue("30"); break;
            case 32: Effects_Type->ChangeValue("31"); break;
            case 33: Effects_Type->ChangeValue("32"); break;
            case 34: Effects_Type->ChangeValue("33"); break;
            case 35: Effects_Type->ChangeValue("34"); break;
            case 36: Effects_Type->ChangeValue("35"); break;
            case 37: Effects_Type->ChangeValue("36"); break;
            case 38: Effects_Type->ChangeValue("37"); break;
            case 39: Effects_Type->ChangeValue("38"); break;
            case 40: Effects_Type->ChangeValue("39"); break;
            case 41: Effects_Type->ChangeValue("101"); break;
            case 42: Effects_Type->ChangeValue("102"); break;
            case 43: Effects_Type->ChangeValue("103"); break;
            default: Effects_Type->ChangeValue("255");
        }
        Effects_Type->SaveEdits();
        ListEffectCmds();
    });
    Effects_A->Bind(wxEVT_KILL_FOCUS, &AGE_Frame::OnKillFocus_Techs, this);
    Effects_A->Bind(wxEVT_TEXT_ENTER, &AGE_Frame::OnEnter_Techs, this);
    Effects_B->Bind(wxEVT_KILL_FOCUS, &AGE_Frame::OnKillFocus_Techs, this);
    Effects_B->Bind(wxEVT_TEXT_ENTER, &AGE_Frame::OnEnter_Techs, this);
    Effects_C->Bind(wxEVT_KILL_FOCUS, &AGE_Frame::OnKillFocus_Techs, this);
    Effects_C->Bind(wxEVT_TEXT_ENTER, &AGE_Frame::OnEnter_Techs, this);
    Effects_D->Bind(wxEVT_KILL_FOCUS, &AGE_Frame::OnKillFocus_Techs, this);
    Effects_D->Bind(wxEVT_TEXT_ENTER, &AGE_Frame::OnEnter_Techs, this);
    Effects_A_ComboBox->Bind(wxEVT_COMBOBOX, &AGE_Frame::OnUpdateCombo_Techs, this);
    Effects_B_CheckBox->Bind(wxEVT_CHECKBOX, &AGE_Frame::OnUpdateCheck_Techs, this);
    Effects_B_ComboBox->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
    {
        // DETECT UP AND PLUSONE!!!
        switch (event.GetSelection())
        {
            case 0: Effects_B->ChangeValue("-1"); break;
            case 1: Effects_B->ChangeValue("-2"); break;
            case 2: Effects_B->ChangeValue("0"); break;
            case 3: Effects_B->ChangeValue("1"); break;
            case 4: Effects_B->ChangeValue("2"); break;
            case 5: Effects_B->ChangeValue("3"); break;
            case 6: Effects_B->ChangeValue("16384"); break;
            case 7: Effects_B->ChangeValue("16385"); break;
            case 8: Effects_B->ChangeValue("16386"); break;
            case 9: Effects_B->ChangeValue("16387"); break;
            default: Effects_B->ChangeValue("-1");
        }
        Effects_B->SaveEdits();
        ListEffectCmds();
    });
    Effects_C_CheckBox->Bind(wxEVT_CHECKBOX, &AGE_Frame::OnUpdateCheck_Techs, this);
    Effects_C_ComboBox->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
    {
        // DETECT EA AND PLUSONE!!!
        int selection = event.GetSelection();
        selection = (selection < 25) ? selection - 1 : selection + 75;
        Effects_C->ChangeValue(lexical_cast<string>(selection));
        Effects_C->SaveEdits();
        ListEffectCmds();
    });
    Effects_D_ComboBox->Bind(wxEVT_COMBOBOX, &AGE_Frame::OnUpdateCombo_Techs, this);
    Effects_89_Type_CB1->Bind(wxEVT_COMBOBOX, [this](wxCommandEvent& event)
    {
        uint16_t Class = (event.GetSelection() - 1) << 8;
        uint8_t Amount = lexical_cast<int>(Effects_89_Amount->GetValue());
        Effects_D->ChangeValue(lexical_cast<string>(Amount + Class));
        Effects_D->SaveEdits();

        ListEffectCmds();
        return;
    });
}

void AGE_Frame::OnEnter_Techs(wxCommandEvent &event)
{
    static_cast<AGETextCtrl*>(event.GetEventObject())->SaveEdits(true);
    ListEffectCmds();
}

void AGE_Frame::OnKillFocus_Techs(wxFocusEvent &event)
{
    event.Skip();
    if (static_cast<AGETextCtrl*>(event.GetEventObject())->SaveEdits() == 0)
    {
        ListEffectCmds();
    }
}

void AGE_Frame::UpdateEffects89(bool forced)
{
    // if has something, then update float value.
    if(!Effects_89_Amount->IsEmpty() && !Effects_89_Type->IsEmpty())
    {
        uint16_t Class = lexical_cast<int>(Effects_89_Type->GetValue()) << 8;
        uint8_t Amount = lexical_cast<int>(Effects_89_Amount->GetValue());
        Effects_D->ChangeValue(lexical_cast<string>(Amount + Class));
        Effects_D->SaveEdits(forced);
    }
    ListEffectCmds();
}

void AGE_Frame::OnEnter_Effects89(wxCommandEvent &event)
{
    UpdateEffects89(true);
}

void AGE_Frame::OnKillFocus_Effects89(wxFocusEvent &event)
{
    event.Skip();
    UpdateEffects89(false);
}

void AGE_Frame::OnUpdateCheck_Techs(wxCommandEvent &event)
{
    static_cast<LinkedCheckBox*>(event.GetEventObject())->OnChoose(event);
    ListEffectCmds();
}

void AGE_Frame::OnUpdateCombo_Techs(wxCommandEvent &event)
{
    static_cast<LinkedComboBox*>(event.GetEventObject())->OnChoose(event);
    ListEffectCmds();
}
