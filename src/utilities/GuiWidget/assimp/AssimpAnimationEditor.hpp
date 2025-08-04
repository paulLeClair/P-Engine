//
// Created by paull on 2025-06-27.
//

#pragma once
#include <bitset>
#include <map>
#include <utility>

#include "../GuiWidget.hpp"
#include "../Table.hpp"
#include "../../assimp/AssimpData.hpp"
#include "../../dear_imgui/imgui.h"

// only including this for the max bones macro
#include "../../../GraphicsEngine/Scene/SceneResources/Model/Model.hpp"


namespace pEngine::util::gui {
    // idea: this is a little window that will contain a traversable ImGui tree
    // for a particular animation. I'm going to just hardcode it for that specific purpose for now
    class AssimpAnimationEditor final : public GuiWidget {
    public:
        enum class ActiveKeyframeFlag {
            UNDEFINED,
            PREVIOUS,
            CURRENT,
            NEXT
        };

        AssimpAnimationEditor(std::string name, assimp::SkeletalAnimation &anim,
                              ActiveKeyframeFlag &currentlyAppliedPose)
            : name(std::move(name)),
              linearizedBones(anim.getLinearizedBones()),
              currentAnimationChannels(anim.animationChannels),
              originalUneditedChannels(anim.animationChannels),
              animationChannelTable(nullptr),
              currentlyAppliedPose(currentlyAppliedPose),
              cacheCopy({}),
              previousFrameInfo({}),
              nextFrameInfo({}) {
            tableRows.clear();
            uint32_t rowId = 0;
            for (auto &[nodeName, animChannel]: anim.animationChannels) {
                tableRows.emplace_back(std::make_unique<AnimationChannelTableRow>(
                    nodeName,
                    &animChannel,
                    this
                ));
                tableRows.back()->id = rowId;
                rowId++;
            }

            // init our disabled row lists
            for (const auto &nodeName: originalUneditedChannels | std::views::keys) {
                disabledPositionKeyframes[nodeName] = {};
                disabledRotationKeyframes[nodeName] = {};
                disabledScalingKeyframes[nodeName] = {};
            }
            currentAnimationChannels = originalUneditedChannels;

            animationChannelTable = std::make_unique<Table>(
                Table::CreationInput{
                    name + " Animation Channel Table",
                    true,
                    12,
                    12,
                    ImGuiTableColumnFlags_None,
                    tableColumns,
                    tableRows,
                    ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable,
                    false
                }
            );
            pendingAnimationUpdates = true;
        }

        void updateCache(assimp::AnimationInfoCache &&newCache) {
            cacheCopy = std::move(newCache);
        }

        std::function<void()> toCallback() override {
            return [&] {
                ImGui::Begin("Animation Viewer");

                // apart from the keyframe readout, we want to also specifically grab a bunch of info about the specific
                // keyframes
                if (cacheDebugInfo) {
                    if (ImGui::BeginMenu("Keyframe Pose Selector")) {
                        if (ImGui::MenuItem("previous")) {
                            currentlyAppliedPose = ActiveKeyframeFlag::PREVIOUS;
                        }
                        if (ImGui::MenuItem("current")) {
                            currentlyAppliedPose = ActiveKeyframeFlag::CURRENT;
                        }
                        if (ImGui::MenuItem("next")) {
                            currentlyAppliedPose = ActiveKeyframeFlag::NEXT;
                        }
                        ImGui::EndMenu();
                    }
                }

                // Display keyframe data for each animated bone

                // NEW: a few new buttons for the animation editor
                if (ImGui::SmallButton("Reset all keyframes")) {
                    currentAnimationChannels = originalUneditedChannels;
                    for (auto &nodeName: std::views::keys(currentAnimationChannels)) {
                        disabledPositionKeyframes[nodeName].clear();
                        disabledRotationKeyframes[nodeName].clear();
                        disabledScalingKeyframes[nodeName].clear();
                    }
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Disable selected keyframes")) {
                    bool rowsHaveBeenSelected = false;

                    for (auto &ptr: tableRows) {
                        auto &row = *(AnimationChannelTableRow *) ptr.get();
                        if (!row.positionKeyframes.selectedRows.empty()) {
                            rowsHaveBeenSelected = true;
                            for (auto &selectedNodeName: row.positionKeyframes.selectedRows) {
                                PositionAndScaleKeyframeTableRow &selectedPosition
                                        = *(PositionAndScaleKeyframeTableRow *) row.positionKeyframes
                                        .rowsHandle[row.positionKeyframes.selectedRowIndices.at(selectedNodeName)]
                                        .get();
                                disabledPositionKeyframes.at(row.nodeName).insert(selectedPosition.timeInTicks);
                            }
                        }
                        if (!row.rotationKeyframes.selectedRows.empty()) {
                            rowsHaveBeenSelected = true;
                            for (auto &selectedNodeName: row.rotationKeyframes.selectedRows) {
                                RotationKeyframeTableRow &selectedRotation
                                        = *(RotationKeyframeTableRow *) row.rotationKeyframes
                                        .rowsHandle[row.rotationKeyframes.selectedRowIndices.at(selectedNodeName)]
                                        .get();
                                disabledRotationKeyframes.at(row.nodeName).insert(selectedRotation.timeInTicks);
                            }
                        }
                        if (!row.scaleKeyframes.selectedRows.empty()) {
                            rowsHaveBeenSelected = true;
                            for (auto &selectedNodeName: row.scaleKeyframes.selectedRows) {
                                PositionAndScaleKeyframeTableRow &selectedScaleKey
                                        = *(PositionAndScaleKeyframeTableRow *) row.scaleKeyframes
                                        .rowsHandle[row.scaleKeyframes.selectedRowIndices.at(selectedNodeName)]
                                        .get();
                                disabledScalingKeyframes.at(row.nodeName).insert(selectedScaleKey.timeInTicks);
                            }
                        }
                    }

                    pendingAnimationUpdates = rowsHaveBeenSelected;
                }

                if (pendingAnimationUpdates) {
                    // simple/ugly method: re-load everything, skipping disabled rows
                    currentAnimationChannels.clear();
                    for (auto &[nodeName, channel]: originalUneditedChannels) {
                        currentAnimationChannels.insert_or_assign(nodeName, assimp::AnimationChannel{
                                                                      .nodeName = nodeName
                                                                  });

                        for (auto &positionKey: channel.positionKeys) {
                            if (!disabledPositionKeyframes[nodeName].contains(positionKey.time)) {
                                currentAnimationChannels.at(nodeName).positionKeys.emplace_back(positionKey);
                            }
                        }
                        for (auto &rotationKey: channel.rotationKeys) {
                            if (!disabledRotationKeyframes[nodeName].contains(rotationKey.time)) {
                                currentAnimationChannels.at(nodeName).rotationKeys.emplace_back(rotationKey);
                            }
                        }
                        for (auto &scaleKey: channel.scaleKeys) {
                            if (!disabledScalingKeyframes[nodeName].contains(scaleKey.time)) {
                                currentAnimationChannels.at(nodeName).scaleKeys.emplace_back(scaleKey);
                            }
                        }
                    }

                    pendingAnimationUpdates = false;
                }
                animationChannelTable->toCallback()();

                ImGui::End();
            };
        }

        std::unordered_map<std::string, assimp::AnimationChannel> currentAnimationChannels = {};
        std::unordered_map<std::string, std::set<double> > disabledPositionKeyframes = {};
        std::unordered_map<std::string, std::set<double> > disabledRotationKeyframes = {};
        std::unordered_map<std::string, std::set<double> > disabledScalingKeyframes = {};

    private:
        std::string name;

        std::vector<assimp::Bone> linearizedBones = {};
        std::map<uint32_t, bool> isLeafNode = {};

        bool pendingAnimationUpdates = false;

        const std::unordered_map<std::string, assimp::AnimationChannel> originalUneditedChannels;

        const std::vector<TableColumnDescription> tableColumns = {
            // just define em here
            TableColumnDescription{
                "Node Name",
                ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthFixed
            },
            TableColumnDescription{
                "Position Keyframes"
            },
            TableColumnDescription{
                "Rotation Keyframes"
            },
            TableColumnDescription{
                "Scale Keyframes"
            }
        };

        struct PositionAndScaleKeyframeTableRow final : TableRow {
            enum TYPE {
                UNDEFINED,
                POSITION,
                SCALE
            };

            const TYPE type = UNDEFINED;

            double timeInTicks;
            glm::vec3 value;

            // new: rows will check if they're disabled and change their BG color if so
            const AssimpAnimationEditor *parentEditor;
            const std::string &nodeName;

            PositionAndScaleKeyframeTableRow(const double time_in_ticks, const glm::vec3 &value,
                                             const AssimpAnimationEditor *parentEditor, const std::string &nodeName,
                                             const TYPE type)
                : type(type), timeInTicks(time_in_ticks), value(value), parentEditor(parentEditor), nodeName(nodeName) {
            }

            void drawRow() override {
                bool rowIsDisabled;
                static constexpr ImVec4 col{1.0f, 0.3, 0.2, 1};
                switch (type) {
                    case POSITION: {
                        rowIsDisabled = parentEditor->disabledPositionKeyframes.at(nodeName).contains(timeInTicks);
                        break;
                    }
                    case SCALE: {
                        rowIsDisabled = parentEditor->disabledScalingKeyframes.at(nodeName).contains(timeInTicks);
                        break;
                    }
                    default:
                        rowIsDisabled = false;
                }
                if (rowIsDisabled) {
                    ImGui::PushStyleColor(ImGuiCol_Text, col);
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(std::to_string(timeInTicks).c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(std::to_string(value.x).c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text(std::to_string(value.y).c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text(std::to_string(value.z).c_str());

                if (rowIsDisabled) {
                    ImGui::PopStyleColor();
                }
            }
        };

        struct RotationKeyframeTableRow final : TableRow {
            double timeInTicks;
            glm::quat value;

            // new: rows will check if they're disabled and change their BG color if so
            const AssimpAnimationEditor *parentEditor;
            const std::string &nodeName;

            RotationKeyframeTableRow(const double time_in_ticks, const glm::quat &value,
                                     const AssimpAnimationEditor *parentEditor,
                                     const std::string &nodeName)
                : timeInTicks(time_in_ticks), value(value), parentEditor(parentEditor), nodeName(nodeName) {
            }

            void drawRow() override {
                const bool isRowDisabled = parentEditor->disabledRotationKeyframes.at(nodeName).contains(timeInTicks);

                static constexpr ImVec4 col{1.0f, 0.3, 0.2, 1};

                if (isRowDisabled) {
                    ImGui::PushStyleColor(ImGuiCol_Text, col);
                }

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(std::to_string(timeInTicks).c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(std::to_string(value.w).c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text(std::to_string(value.x).c_str());
                ImGui::TableSetColumnIndex(3);
                ImGui::Text(std::to_string(value.y).c_str());
                ImGui::TableSetColumnIndex(4);
                ImGui::Text(std::to_string(value.z).c_str());

                if (isRowDisabled) {
                    ImGui::PopStyleColor();
                }
            }
        };

        struct AnimationChannelTableRow final : TableRow {
            const std::vector<TableColumnDescription> positionAndScaleColumnDescriptions =
            {
                TableColumnDescription{
                    .label = "Time (ticks)",
                    .flags = ImGuiTableColumnFlags_None
                },
                TableColumnDescription{
                    .label = "x"
                },
                TableColumnDescription{
                    .label = "y"
                },
                TableColumnDescription{
                    .label = "z"
                }
            };
            const std::vector<TableColumnDescription> rotationColumnDescriptions = {
                TableColumnDescription{
                    .label = "Time (ticks)",
                    .flags = ImGuiTableColumnFlags_None
                },
                TableColumnDescription{
                    .label = "w"
                },
                TableColumnDescription{
                    .label = "x"
                },
                TableColumnDescription{
                    .label = "y"
                },
                TableColumnDescription{
                    .label = "z"
                }
            };

            // idea:
            // 1. one row for each anim channel (node anim)
            // 2. each row has a collapsible nested table of animation data for each track (can modify/port over old one)
            // COMPLICATION: the nested tables here maybe; hopefully it's not too brutal
            std::string nodeName;
            assimp::AnimationChannel *channel;

            // NEW: each row has its own Table widgets for each keyframe track
            std::vector<std::unique_ptr<TableRow> > positionKeyframeRows = {};
            Table positionKeyframes;
            std::vector<std::unique_ptr<TableRow> > rotationKeyframeRows = {};
            Table rotationKeyframes;
            std::vector<std::unique_ptr<TableRow> > scaleKeyframeRows = {};
            Table scaleKeyframes;

            AnimationChannelTableRow(std::string name,
                                     assimp::AnimationChannel *channel,
                                     const AssimpAnimationEditor *parentEditor)
                : nodeName(std::move(name)),
                  channel(channel),
                  positionKeyframeRows(std::move(obtainPositionKeyframeRows(channel, parentEditor))),
                  positionKeyframes(Table::CreationInput{
                      name + " Animation Channel Position Keyframes Table",
                      false,
                      12,
                      12,
                      ImGuiTableColumnFlags_None,
                      positionAndScaleColumnDescriptions,
                      positionKeyframeRows,
                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable,
                      true
                  }),
                  rotationKeyframeRows(std::move(obtainRotationKeyframeRows(channel, parentEditor))),
                  rotationKeyframes(Table::CreationInput{
                      name + " Animation Channel Rotation Keyframes Table",
                      false,
                      12,
                      12,
                      ImGuiTableColumnFlags_None,
                      rotationColumnDescriptions,
                      rotationKeyframeRows,
                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable,
                      true
                  }),
                  scaleKeyframeRows(std::move(obtainScaleKeyframeRows(channel, parentEditor))),
                  scaleKeyframes(Table::CreationInput{
                      name + " Animation Channel Scaling Keyframes Table",
                      false,
                      12,
                      12,
                      ImGuiTableColumnFlags_None,
                      positionAndScaleColumnDescriptions,
                      scaleKeyframeRows,
                      ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable,
                      true
                  }),
                  parentEditor(parentEditor) {
            }

            ~AnimationChannelTableRow() override = default;

            void drawRow() override {
                // TODO -> add a little edit button per-row

                ImGui::TableSetColumnIndex(0);
                ImGui::Text(nodeName.c_str());

                ImGui::TableSetColumnIndex(1);
                displayPositionAnimationTrack(channel);

                ImGui::TableSetColumnIndex(2);
                displayRotationAnimationTrack(channel);

                ImGui::TableSetColumnIndex(3);
                displayScaleAnimationTrack(channel);
            }

        private:
            void displayPositionAnimationTrack(const assimp::AnimationChannel *channel) {
                if (!channel) {
                    // TODO -> log!
                    return;
                }

                if (ImGui::TreeNode("Position Keyframes")) {
                    // NEW -> display the position keyframe Table widget
                    positionKeyframes.toCallback()();
                    ImGui::TreePop();
                }
            }

            void displayRotationAnimationTrack(const assimp::AnimationChannel *channel) {
                if (!channel) {
                    // TODO -> log!
                    return;
                }
                if (ImGui::TreeNode("Rotation Keyframes")) {
                    // NEW -> display the rotation keyframe Table widget
                    rotationKeyframes.toCallback()();
                    ImGui::TreePop();
                }
            }

            void displayScaleAnimationTrack(const assimp::AnimationChannel *channel) {
                if (!channel) {
                    // TODO -> log!
                    return;
                }
                if (ImGui::TreeNode("Scale Keyframes")) {
                    // NEW -> display the rotation keyframe Table widget
                    scaleKeyframes.toCallback()();
                    ImGui::TreePop();
                }
            }

            const AssimpAnimationEditor *parentEditor;

            static std::vector<std::unique_ptr<TableRow> >
            obtainPositionKeyframeRows(const assimp::AnimationChannel *channel,
                                       const AssimpAnimationEditor *parentEditor) {
                std::vector<std::unique_ptr<TableRow> > rows;
                uint32_t rowIndex = 0;
                for (const auto &[time, position]: channel->positionKeys) {
                    rows.emplace_back(std::make_unique<PositionAndScaleKeyframeTableRow>(
                        time,
                        position,
                        parentEditor,
                        channel->nodeName,
                        PositionAndScaleKeyframeTableRow::TYPE::POSITION
                    ));
                    rows.back()->id = std::string(channel->nodeName + "_position_keyframe_" + std::to_string(rowIndex)).
                            c_str();
                    rowIndex++;
                }
                return rows;
            }

            static std::vector<std::unique_ptr<TableRow> >
            obtainRotationKeyframeRows(const assimp::AnimationChannel *channel,
                                       const AssimpAnimationEditor *parentEditor) {
                std::vector<std::unique_ptr<TableRow> > rows;
                uint32_t rowIndex = 0;
                for (const auto &[time, rotation]: channel->rotationKeys) {
                    rows.emplace_back(std::make_unique<RotationKeyframeTableRow>(
                        time,
                        rotation,
                        parentEditor,
                        channel->nodeName
                    ));
                    rows.back()->id = std::string(channel->nodeName + "_rotation_keyframe_" + std::to_string(rowIndex)).
                            c_str();
                    rowIndex++;
                }
                return rows;
            }

            static std::vector<std::unique_ptr<TableRow> > obtainScaleKeyframeRows(
                const assimp::AnimationChannel *channel, const AssimpAnimationEditor *parentEditor) {
                std::vector<std::unique_ptr<TableRow> > rows;
                uint32_t rowIndex = 0;
                for (const auto &[time, scale]: channel->scaleKeys) {
                    rows.emplace_back(std::make_unique<PositionAndScaleKeyframeTableRow>(
                        time,
                        scale,
                        parentEditor,
                        channel->nodeName,
                        PositionAndScaleKeyframeTableRow::TYPE::SCALE
                    ));
                    rows.back()->id = std::string(channel->nodeName + "_scale_keyframe_" + std::to_string(rowIndex)).
                            c_str();
                    rowIndex++;
                }
                return rows;
            }
        };

        std::vector<std::unique_ptr<TableRow> > tableRows = {};
        std::unique_ptr<Table> animationChannelTable{};

        // external handle to a living keyframe variable that will be used to send over the appropriate keyframe poses
        bool cacheDebugInfo = false;
        ActiveKeyframeFlag &currentlyAppliedPose;
        // NEW APPROACH: just copy the whole damn thing over each frame...
        // assimp::AnimationInfoCache *cacheHandle;
        assimp::AnimationInfoCache cacheCopy = {};

        double currentTimeInTicks = 0;
        assimp::KeyFrameInfo previousFrameInfo;
        assimp::KeyFrameInfo nextFrameInfo;
    };
}
