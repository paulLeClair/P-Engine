//
// Created by paull on 2025-07-07.
//

#pragma once

#include "../../lib/dear_imgui/imgui.h"

#include "GuiWidget.hpp"

namespace pEngine::util::gui {
    struct TableRow {
        virtual ~TableRow() = default;

        std::string id;

        virtual void drawRow() = 0;
    };

    struct TableColumnDescription {
        // std::string label;
        const char *label;
        ImGuiTableColumnFlags flags;
    };

    class Table final : public GuiWidget {
    public:
        struct CreationInput {
            std::string name;

            bool showOptionsMenu = true;

            uint32_t textBaseWidth;
            uint32_t textBaseHeight;

            ImGuiTableColumnFlags baseColumnFlags = ImGuiTableColumnFlags_None;

            std::vector<TableColumnDescription> columnDescriptions;

            std::vector<std::unique_ptr<TableRow> > &rowsHandle;

            ImGuiTableFlags baseFlags = ImGuiTableFlags_None;

            bool enableClipper = true;
        };

        /**
         * This is a "widgetization" of the demo advanced table. It's intended to be general purpose, but
         * off the bat it'll be kinda janky I'm sure.
         * @param input
         */
        explicit Table(const CreationInput &input) : name(input.name), columnFlags(input.baseColumnFlags),
                                                     flags(input.baseFlags),
                                                     showOptionsMenu(input.showOptionsMenu),
                                                     textBaseWidth(input.textBaseWidth),
                                                     textBaseHeight(input.textBaseHeight),
                                                     columnDescriptions(input.columnDescriptions),
                                                     rowsHandle(input.rowsHandle),
                                                     enableClipper(input.enableClipper) {
        }


        ~Table() override = default;

        std::function<void()> toCallback() override {
            // should probably change this to be a set and use string labels;
            return [&]() {
                if (showOptionsMenu) displayOptionsMenu();

                // from here on we would display our actual table;
                if (ImGui::BeginTable(
                    name.c_str(),
                    static_cast<uint32_t>(columnDescriptions.size()),
                    flags,
                    outer_size_enabled ? outer_size_value : ImVec2(0, 0),
                    (flags & ImGuiTableFlags_ScrollX) ? inner_width_with_scroll : 0.0f
                )) {
                    uint32_t columnId = 0;
                    for (const auto &column: columnDescriptions) {
                        ImGui::TableSetupColumn(
                            column.label,
                            columnFlags,
                            0.0f,
                            columnId
                        );
                        columnId++;
                    }

                    if (show_headers && (columnFlags & ImGuiTableColumnFlags_AngledHeader) != 0) {
                        ImGui::TableAngledHeadersRow();
                    }
                    if (show_headers) {
                        ImGui::TableHeadersRow();
                    }
                    if (!rowsHandle.empty()) {
                        // clipper for large vertical lists (DISABLED FOR NOW)
                        if (enableClipper) {
                            ImGuiListClipper clipper;
                            clipper.Begin(rowsHandle.size());
                            while (clipper.Step()) {
                                int32_t rowIndex = 0;
                                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                                    TableRow *row = rowsHandle[row_n].get();

                                    const bool rowIsSelected = selectedRows.contains(row->id);
                                    ImGui::PushID(row->id.c_str()); // maybe unneeded if sorting is off
                                    ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);

                                    // NOTE: for selectable, we have to call table next column (should hopefully be overwritten
                                    // inside of the drawRow() callback)
                                    ImGui::TableNextColumn();
                                    // for now, we'll assume that every row is a selectable span type thing; we'll change that tho
                                    constexpr ImGuiSelectableFlags selectable_flags =
                                            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
                                    if (ImGui::Selectable(("##selectableRow" + std::to_string(rowIndex)).c_str(),
                                                          rowIsSelected, selectable_flags,
                                                          ImVec2(0, row_min_height))) {
                                        if (ImGui::GetIO().KeyCtrl) {
                                            if (rowIsSelected) {
                                                selectedRows.erase(row->id);
                                                selectedRowIndices.erase(row->id);
                                            } else {
                                                selectedRows.insert(row->id);
                                                selectedRowIndices[row->id] = rowIndex;
                                            }
                                        } else {
                                            selectedRows.clear();
                                            selectedRows.insert(row->id);
                                            selectedRowIndices[row->id] = rowIndex;
                                        }
                                    }

                                    ImGui::SameLine(); // try this to make the row's label align with the selectable
                                    row->drawRow();

                                    ImGui::PopID();
                                    rowIndex++;
                                }
                            }
                        } else {
                            int32_t rowIndex = 0;
                            for (const auto &row_n: rowsHandle) {
                                TableRow *row = row_n.get();

                                const bool rowIsSelected = selectedRows.contains(row->id);
                                ImGui::PushID(row->id.c_str()); // maybe unneeded if sorting is off
                                ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);

                                // NOTE: for selectable, we have to call table next column (should hopefully be overwritten
                                // inside of the drawRow() callback)
                                ImGui::TableNextColumn();
                                // for now, we'll assume that every row is a selectable span type thing; we'll change that tho
                                constexpr ImGuiSelectableFlags selectable_flags =
                                        ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap;
                                if (ImGui::Selectable(("##selectableRow" + std::to_string(rowIndex)).c_str(),
                                                      rowIsSelected, selectable_flags,
                                                      ImVec2(0, row_min_height))) {
                                    if (ImGui::GetIO().KeyCtrl) {
                                        if (rowIsSelected) {
                                            selectedRows.erase(row->id);
                                            selectedRowIndices.erase(row->id);
                                        } else {
                                            selectedRows.insert(row->id);
                                            selectedRowIndices[row->id] = rowIndex;
                                        }
                                    } else {
                                        selectedRows.clear();
                                        selectedRows.insert(row->id);
                                        selectedRowIndices[row->id] = rowIndex;
                                    }
                                }

                                ImGui::SameLine(); // try this to make the row's label be aligned with the selectable
                                row->drawRow();

                                ImGui::PopID();
                                rowIndex++;
                            }
                        }
                    }
                    ImGui::EndTable();
                }
            };
        }

        std::string name;

        ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
                                // | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
                                | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBody
                                | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY
                                | ImGuiTableFlags_SizingFixedFit;
        ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_None;

        std::set<std::string> selectedRows;
        std::map<std::string, uint32_t> selectedRowIndices;

        bool showOptionsMenu;
        uint32_t textBaseWidth;
        uint32_t textBaseHeight;

        bool outer_size_enabled = false;
        bool show_headers = true;
        bool show_wrapped_text = false;

        int freeze_cols = 1;
        int freeze_rows = 1;
        float row_min_height = 0.0f; // Auto
        float inner_width_with_scroll = 0.0f; // Auto-extend

        ImVec2 outer_size_value = ImVec2(0.0f, textBaseHeight * 12);

        std::vector<TableColumnDescription> columnDescriptions;

        std::vector<std::unique_ptr<TableRow> > &rowsHandle;
        bool enableClipper;

    private:
        void displayOptionsMenu() {
            if (ImGui::TreeNode("Options")) {
                // // Make the UI compact because there are so many fields
                PushStyleCompact();
                ImGui::PushItemWidth(textBaseWidth * 28.0f);

                if (ImGui::TreeNodeEx("Features:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::CheckboxFlags("ImGuiTableFlags_Resizable", &flags, ImGuiTableFlags_Resizable);
                    ImGui::CheckboxFlags("ImGuiTableFlags_Reorderable", &flags, ImGuiTableFlags_Reorderable);
                    ImGui::CheckboxFlags("ImGuiTableFlags_Hideable", &flags, ImGuiTableFlags_Hideable);
                    ImGui::CheckboxFlags("ImGuiTableFlags_Sortable", &flags, ImGuiTableFlags_Sortable);
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoSavedSettings", &flags, ImGuiTableFlags_NoSavedSettings);
                    ImGui::CheckboxFlags("ImGuiTableFlags_ContextMenuInBody", &flags,
                                         ImGuiTableFlags_ContextMenuInBody);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Decorations:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::CheckboxFlags("ImGuiTableFlags_RowBg", &flags, ImGuiTableFlags_RowBg);
                    ImGui::CheckboxFlags("ImGuiTableFlags_BordersV", &flags, ImGuiTableFlags_BordersV);
                    ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterV", &flags, ImGuiTableFlags_BordersOuterV);
                    ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerV", &flags, ImGuiTableFlags_BordersInnerV);
                    ImGui::CheckboxFlags("ImGuiTableFlags_BordersH", &flags, ImGuiTableFlags_BordersH);
                    ImGui::CheckboxFlags("ImGuiTableFlags_BordersOuterH", &flags, ImGuiTableFlags_BordersOuterH);
                    ImGui::CheckboxFlags("ImGuiTableFlags_BordersInnerH", &flags, ImGuiTableFlags_BordersInnerH);
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBody", &flags, ImGuiTableFlags_NoBordersInBody);
                    ImGui::SameLine();
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoBordersInBodyUntilResize", &flags,
                                         ImGuiTableFlags_NoBordersInBodyUntilResize);
                    // ImGui::SameLine();
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Sizing:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    editTableSizingFlags(&flags);
                    ImGui::SameLine();
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendX", &flags, ImGuiTableFlags_NoHostExtendX);
                    ImGui::SameLine();
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoHostExtendY", &flags, ImGuiTableFlags_NoHostExtendY);
                    ImGui::SameLine();
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoKeepColumnsVisible", &flags,
                                         ImGuiTableFlags_NoKeepColumnsVisible);
                    ImGui::SameLine();

                    ImGui::CheckboxFlags("ImGuiTableFlags_PreciseWidths", &flags, ImGuiTableFlags_PreciseWidths);
                    ImGui::SameLine();
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoClip", &flags, ImGuiTableFlags_NoClip);
                    // ImGui::SameLine();
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Padding:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::CheckboxFlags("ImGuiTableFlags_PadOuterX", &flags, ImGuiTableFlags_PadOuterX);
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoPadOuterX", &flags, ImGuiTableFlags_NoPadOuterX);
                    ImGui::CheckboxFlags("ImGuiTableFlags_NoPadInnerX", &flags, ImGuiTableFlags_NoPadInnerX);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Scrolling:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::CheckboxFlags("ImGuiTableFlags_ScrollX", &flags, ImGuiTableFlags_ScrollX);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
                    ImGui::DragInt("freeze_cols", &freeze_cols, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
                    ImGui::CheckboxFlags("ImGuiTableFlags_ScrollY", &flags, ImGuiTableFlags_ScrollY);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(ImGui::GetFrameHeight());
                    ImGui::DragInt("freeze_rows", &freeze_rows, 0.2f, 0, 9, NULL, ImGuiSliderFlags_NoInput);
                    ImGui::TreePop();
                }
                // Sorting disabled for now;
                // if (ImGui::TreeNodeEx("Sorting:", ImGuiTreeNodeFlags_DefaultOpen)) {
                //     ImGui::CheckboxFlags("ImGuiTableFlags_SortMulti", &flags, ImGuiTableFlags_SortMulti);
                //     ImGui::SameLine();
                //     ImGui::CheckboxFlags("ImGuiTableFlags_SortTristate", &flags, ImGuiTableFlags_SortTristate);
                //     ImGui::SameLine();
                //     ImGui::TreePop();
                // }

                if (ImGui::TreeNodeEx("Headers:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("show_headers", &show_headers);
                    ImGui::CheckboxFlags("ImGuiTableFlags_HighlightHoveredColumn", &flags,
                                         ImGuiTableFlags_HighlightHoveredColumn);
                    ImGui::CheckboxFlags("ImGuiTableColumnFlags_AngledHeader", &columnFlags,
                                         ImGuiTableColumnFlags_AngledHeader);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Other:", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Checkbox("show_wrapped_text", &show_wrapped_text);

                    ImGui::DragFloat2("##OuterSize", &outer_size_value.x);
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    ImGui::Checkbox("outer_size", &outer_size_enabled);
                    ImGui::SameLine();

                    // From a user point of view we will tend to use 'inner_width' differently depending on whether our table is embedding scrolling.
                    // To facilitate toying with this demo we will actually pass 0.0f to the BeginTable() when ScrollX is disabled.
                    ImGui::DragFloat("inner_width (when ScrollX active)", &inner_width_with_scroll, 1.0f, 0.0f,
                                     FLT_MAX);

                    ImGui::DragFloat("row_min_height", &row_min_height, 1.0f, 0.0f, FLT_MAX);

                    ImGui::TreePop();
                }

                ImGui::PopItemWidth();
                PopStyleCompact();
                ImGui::Spacing();
                ImGui::TreePop();
            }
        }

        static void editTableSizingFlags(ImGuiTableFlags *p_flags) {
            struct EnumDesc {
                ImGuiTableFlags Value;
                const char *Name;
                const char *Tooltip;
            };
            static const EnumDesc policies[] =
            {
                {
                    ImGuiTableFlags_None, "Default",
                    "Use default sizing policy:\n- ImGuiTableFlags_SizingFixedFit if ScrollX is on or if host window has ImGuiWindowFlags_AlwaysAutoResize.\n- ImGuiTableFlags_SizingStretchSame otherwise."
                },
                {
                    ImGuiTableFlags_SizingFixedFit, "ImGuiTableFlags_SizingFixedFit",
                    "Columns default to _WidthFixed (if resizable) or _WidthAuto (if not resizable), matching contents width."
                },
                {
                    ImGuiTableFlags_SizingFixedSame, "ImGuiTableFlags_SizingFixedSame",
                    "Columns are all the same width, matching the maximum contents width.\nImplicitly disable ImGuiTableFlags_Resizable and enable ImGuiTableFlags_NoKeepColumnsVisible."
                },
                {
                    ImGuiTableFlags_SizingStretchProp, "ImGuiTableFlags_SizingStretchProp",
                    "Columns default to _WidthStretch with weights proportional to their widths."
                },
                {
                    ImGuiTableFlags_SizingStretchSame, "ImGuiTableFlags_SizingStretchSame",
                    "Columns default to _WidthStretch with same weights."
                }
            };
            int idx;
            for (idx = 0; idx < IM_ARRAYSIZE(policies); idx++)
                if (policies[idx].Value == (*p_flags & ImGuiTableFlags_SizingMask_))
                    break;
            const char *preview_text = (idx < IM_ARRAYSIZE(policies))
                                           ? policies[idx].Name + (idx > 0 ? strlen("ImGuiTableFlags") : 0)
                                           : "";
            if (ImGui::BeginCombo("Sizing Policy", preview_text)) {
                for (int n = 0; n < IM_ARRAYSIZE(policies); n++)
                    if (ImGui::Selectable(policies[n].Name, idx == n))
                        *p_flags = (*p_flags & ~ImGuiTableFlags_SizingMask_) | policies[n].Value;
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::BeginItemTooltip()) {
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);
                for (int m = 0; m < IM_ARRAYSIZE(policies); m++) {
                    ImGui::Separator();
                    ImGui::Text("%s:", policies[m].Name);
                    ImGui::Separator();
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().IndentSpacing * 0.5f);
                    ImGui::TextUnformatted(policies[m].Tooltip);
                }
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }


        static void PushStyleCompact() {
            ImGuiStyle &style = ImGui::GetStyle();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                                ImVec2(style.FramePadding.x, (float) (int) (style.FramePadding.y * 0.60f)));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                                ImVec2(style.ItemSpacing.x, (float) (int) (style.ItemSpacing.y * 0.60f)));
        }

        static void PopStyleCompact() {
            ImGui::PopStyleVar(2);
        }
    };
}
