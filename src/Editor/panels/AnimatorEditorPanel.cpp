#include "AnimatorEditorPanel.h"

#include <algorithm>
#include <cstring>
#include <cstdio>

namespace Lengine
{

    // ─────────────────────────────────────────────────────────────────────────
    //  Private accessors  (registry-based, no raw pointers)
    // ─────────────────────────────────────────────────────────────────────────

    AnimationComponent* AnimatorEditorPanel::GetComponent() const
    {
        if (m_Entity == NullEntity || !registry.animations.Has(m_Entity))
            return nullptr;
        return &registry.animations.Get(m_Entity);
    }

    AnimatorController* AnimatorEditorPanel::GetController() const
    {
        AnimationComponent* comp = GetComponent();
        return comp ? &comp->animator : nullptr;
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Construction / destruction
    // ─────────────────────────────────────────────────────────────────────────

    AnimatorEditorPanel::AnimatorEditorPanel(Registry& reg, AssetManager& assets, std::string title)
        : m_Title(std::move(title)), registry(reg), assetManager(assets)
    {
        m_NodesCtx = ImNodes::EditorContextCreate();
    }


    AnimatorEditorPanel::~AnimatorEditorPanel()
    {
        if (m_NodesCtx)
            ImNodes::EditorContextFree(m_NodesCtx);
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Public API
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::SetTarget(const Entity entity)
    {
        m_Entity = entity;
        m_SelectedState = -1;
        m_SelectedLink = -1;
        m_SelectedNode = -1;
        m_NeedsLayout = true;
        m_Open = (GetComponent() != nullptr);

        m_RenameTargetState = -1;
        m_RenameBuffer[0] = '\0';

        m_RenameTargetNode = -1;
        m_NodeRenameBuffer[0] = '\0';
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Top-level render
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::OnImGuiRender()
    {
        if (!m_Open || !GetController())
            return;

        ImGui::SetNextWindowSize(ImVec2(1100, 650), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin(m_Title.c_str(), &m_Open, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::End();
            return;
        }

        RenderToolbar();
        ImGui::Separator();

        // Three-column layout: params | graph | inspector
        const float sideW = 210.0f;
        const float avail = ImGui::GetContentRegionAvail().x;
        const float graphW = avail - sideW * 2.0f - ImGui::GetStyle().ItemSpacing.x * 2.0f;
        const float panelH = ImGui::GetContentRegionAvail().y;

        // ── Left sidebar ─────────────────────────────────────────────────────
        ImGui::BeginChild("##params_sidebar", ImVec2(sideW, panelH), true);
        RenderParameterSidebar();
        ImGui::EndChild();

        ImGui::SameLine();

        // ── Centre: node graph ───────────────────────────────────────────────
        ImGui::BeginChild("##graph_canvas", ImVec2(graphW, panelH), true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        RenderGraphCanvas();
        ImGui::EndChild();

        ImGui::SameLine();

        // ── Right sidebar ────────────────────────────────────────────────────
        ImGui::BeginChild("##inspector_sidebar", ImVec2(sideW, panelH), true);
        RenderInspectorSidebar();
        ImGui::EndChild();

        ImGui::End();

        RenderBlendNodesPanel();
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Toolbar
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::RenderToolbar()
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        if (ImGui::Button("+ State"))
            AddDefaultState();

        ImGui::SameLine();

        bool hasSelection = (m_SelectedState >= 0 || m_SelectedLink >= 0);
        if (!hasSelection) ImGui::BeginDisabled();
        if (ImGui::Button("Delete Selected"))
        {
            DeleteSelectedNodes();
            DeleteSelectedLinks();
        }
        if (!hasSelection) ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Checkbox("Blend Nodes Panel", &m_ShowBlendNodesPanel);

        if (ctrl->currentState >= 0 &&
            ctrl->currentState < (int)ctrl->states.size())
        {
            ImGui::SameLine();
            ImGui::TextDisabled("Default: %s",
                ctrl->states[ctrl->currentState].name.c_str());
        }

        if (m_SelectedState >= 0)
        {
            ImGui::SameLine();
            if (ImGui::Button("Set as Default"))
                ctrl->SetDefaultState(m_SelectedState);
        }
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Parameter sidebar
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::RenderParameterSidebar()
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        // delete queued params
        {
            while (!floatParamsToBeDeleted.empty()) {
                auto param = floatParamsToBeDeleted.front();

                if (ctrl->floatParams.find(param) != ctrl->floatParams.end())
                    ctrl->floatParams.erase(param);

                floatParamsToBeDeleted.pop();
            }

            while (!boolParamsToBeDeleted.empty()) {
                auto param = boolParamsToBeDeleted.front();

                if (ctrl->boolParams.find(param) != ctrl->boolParams.end())
                    ctrl->boolParams.erase(param);

                boolParamsToBeDeleted.pop();
            }
        }
       

        ImGui::TextDisabled("PARAMETERS");
        ImGui::Separator();

        // ── Float params ─────────────────────────────────────────────────────
        for (auto& [name, val] : ctrl->floatParams)
        {
            ImGui::PushID(name.c_str());

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.35f, 0.75f, 1.0f, 1.0f));
            ImGui::TextUnformatted("f");
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::SetNextItemWidth(-1);
            ImGui::DragFloat(("##" + name).c_str(), &val, 0.01f);

            ImGui::TextDisabled("%s", name.c_str());

            if (ImGui::BeginPopupContextItem("##ctx"))
            {
                if (ImGui::MenuItem("Delete"))
                    floatParamsToBeDeleted.push(name);
                ImGui::EndPopup();
            }

            ImGui::Spacing();
            ImGui::PopID();
        }

        // ── Bool params ──────────────────────────────────────────────────────
        for (auto& [name, val] : ctrl->boolParams)
        {
            ImGui::PushID(name.c_str());

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
            ImGui::TextUnformatted("b");
            ImGui::PopStyleColor();
            ImGui::SameLine();

            ImGui::Checkbox(("##" + name).c_str(), &val);
            ImGui::SameLine();
            ImGui::TextUnformatted(name.c_str());

            if (ImGui::BeginPopupContextItem("##ctx"))
            {
                if (ImGui::MenuItem("Delete"))
                    boolParamsToBeDeleted.push(name);
                ImGui::EndPopup();
            }

            ImGui::Spacing();
            ImGui::PopID();
        }

        ImGui::Separator();

        // ── Add new parameter ────────────────────────────────────────────────
        ImGui::SetNextItemWidth(100.0f);
        ImGui::InputText("##newpname", m_NewParamName, sizeof(m_NewParamName));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(52.0f);
        const char* types[] = { "float", "bool" };
        ImGui::Combo("##npt", &m_NewParamType, types, 2);

        if (ImGui::Button("Add##param") && m_NewParamName[0] != '\0')
        {
            std::string n(m_NewParamName);
            if (m_NewParamType == 0)
                ctrl->floatParams.emplace(n, 0.0f);
            else
                ctrl->boolParams.emplace(n, false);
            m_NewParamName[0] = '\0';
        }
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Graph canvas
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::RenderGraphCanvas()
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        ImNodes::EditorContextSet(m_NodesCtx);

        ImNodes::PushColorStyle(ImNodesCol_GridBackground, IM_COL32(28, 28, 30, 255));
        ImNodes::PushColorStyle(ImNodesCol_GridLine, IM_COL32(55, 55, 58, 255));

        ImNodes::BeginNodeEditor();

        if (m_NeedsLayout)
        {
            InitNodePositions();
            m_NeedsLayout = false;
        }

        DrawAnyStateNode();

        for (int i = 0; i < (int)ctrl->states.size(); i++)
            DrawStateNode(i);

        DrawLinks();

        ImNodes::EndNodeEditor();

        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();

        // ── Handle new link drag ─────────────────────────────────────────────
        int fromPin, toPin;
        if (ImNodes::IsLinkCreated(&fromPin, &toPin))
        {
            int fromState = (fromPin == k_AnyStateOutPin) ? -1 : NodeFromOutputPin(fromPin);
            int toState = NodeFromInputPin(toPin);

            if (toState >= 0 && toState < (int)ctrl->states.size() &&
                toState != fromState)
            {
                ctrl->AddTransition(fromState, toState, 0.2f);
            }
        }

        // ── Handle link deletion ─────────────────────────────────────────────
        int destroyedLink;
        if (ImNodes::IsLinkDestroyed(&destroyedLink))
        {
            if (destroyedLink >= 0 &&
                destroyedLink < (int)ctrl->transitions.size())
            {
                ctrl->transitions.erase(ctrl->transitions.begin() + destroyedLink);

                if (m_SelectedLink == destroyedLink)
                    m_SelectedLink = -1;
            }
        }

        if (ImNodes::NumSelectedNodes() == 1)
        {
            int sel = -1;
            ImNodes::GetSelectedNodes(&sel);
            if (sel != k_AnyStateNodeID)
            {
                m_SelectedState = sel;
                m_SelectedLink = -1;
                m_SelectedNode = -1;          // NEW
                m_RenameTargetState = -1;
            }
        }
        else if (ImNodes::NumSelectedNodes() == 0)
        {
            m_SelectedState = -1;
            m_RenameTargetState = -1;
        }

        if (ImNodes::NumSelectedLinks() == 1)
        {
            ImNodes::GetSelectedLinks(&m_SelectedLink);
            m_SelectedState = -1;
            m_SelectedNode = -1;              // NEW
        }
        else if (ImNodes::NumSelectedLinks() == 0 && m_SelectedState < 0)
        {
            m_SelectedLink = -1;
        }
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Individual node drawing
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::DrawStateNode(int stateIdx)
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        auto& state = ctrl->states[stateIdx];
        auto& node = ctrl->nodes[state.rootNodeIndex];

        const bool isDefault = (ctrl->currentState == stateIdx);

        ImVec4 c = BlendTypeColor(node.type);
        ImNodes::PushColorStyle(ImNodesCol_TitleBar,
            IM_COL32((int)(c.x * 255), (int)(c.y * 255), (int)(c.z * 255), 200));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered,
            IM_COL32((int)(c.x * 255), (int)(c.y * 255), (int)(c.z * 255), 255));

        if (isDefault)
            ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(46, 204, 113, 255));

        ImNodes::BeginNode(stateIdx);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(state.name.c_str());
        ImGui::SameLine(0, 6);
        ImGui::TextDisabled("[%s]", BlendTypeLabel(node.type));
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(InputPin(stateIdx));
        ImGui::Dummy(ImVec2(6, 0));
        ImNodes::EndInputAttribute();

        ImGui::PushItemWidth(160.0f);
        switch (node.type)
        {
        case BlendNodeType::Clip:
            ImGui::TextDisabled("clip: %s", ClipName(node.clipID));
            ImGui::TextDisabled(node.looping ? "looping" : "once");
            break;
        case BlendNodeType::Blend1D:
            ImGui::TextDisabled("param: %s", node.parameterName.c_str());
            ImGui::TextDisabled("%d entries", (int)node.blend1DEntries.size());
            break;
        case BlendNodeType::Masked:
            ImGui::TextDisabled("base:    node %d", node.baseNodeIndex);
            ImGui::TextDisabled("overlay: node %d", node.overlayNodeIndex);
            break;
        }
        ImGui::PopItemWidth();

        ImNodes::BeginOutputAttribute(OutputPin(stateIdx));
        ImGui::Dummy(ImVec2(6, 0));
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        if (isDefault)
            ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    void AnimatorEditorPanel::DrawAnyStateNode()
    {
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(160, 100, 20, 200));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(186, 117, 23, 255));

        ImNodes::BeginNode(k_AnyStateNodeID);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("Any State");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginOutputAttribute(k_AnyStateOutPin);
        ImGui::TextDisabled("fires from any");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    void AnimatorEditorPanel::DrawLinks()
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        for (int i = 0; i < (int)ctrl->transitions.size(); i++)
        {
            auto& t = ctrl->transitions[i];

            int outPin = (t.fromState == -1) ? k_AnyStateOutPin : OutputPin(t.fromState);
            int inPin = InputPin(t.toState);

            if (t.fromState == -1)
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(186, 117, 23, 200));

            ImNodes::Link(i, outPin, inPin);

            if (t.fromState == -1)
                ImNodes::PopColorStyle();
        }
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Inspector sidebar
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::RenderInspectorSidebar()
    {
        auto* ctrl = GetController();

        ImGui::TextDisabled("INSPECTOR");
        ImGui::Separator();

        if (!ctrl)
        {
            ImGui::TextDisabled("No animation component");
            return;
        }

        if (m_SelectedState >= 0 &&
            m_SelectedState < (int)ctrl->states.size())
        {
            InspectState(m_SelectedState);
        }
        else if (m_SelectedNode >= 0 &&
            m_SelectedNode < (int)ctrl->nodes.size())
        {
            InspectNode(m_SelectedNode);
        }
        else if (m_SelectedLink >= 0 &&
            m_SelectedLink < (int)ctrl->transitions.size())
        {
            InspectTransition(m_SelectedLink);
        }
        else
        {
            ImGui::TextDisabled("Click a state, node, or transition");
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Blend Nodes panel — every raw BlendNode, draggable by index
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::RenderBlendNodesPanel()
    {
        if (!m_ShowBlendNodesPanel) return;

        auto* ctrl = GetController();
        if (!ctrl) return;

        ImGui::SetNextWindowSize(ImVec2(300, 440), ImGuiCond_FirstUseEver);
        if (!ImGui::Begin("Blend Nodes", &m_ShowBlendNodesPanel))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("+ Node"))
        {
            int idx = ctrl->AddNode(BlendNode::MakeClip(UUID::Null, true));
            m_SelectedNode = idx;
            m_SelectedState = -1;
            m_SelectedLink = -1;
            m_RenameTargetNode = -1;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("(%d total)", (int)ctrl->nodes.size());

        ImGui::TextDisabled("Drag a row onto a Masked node's base/overlay slot");
        ImGui::Separator();

        for (int i = 0; i < (int)ctrl->nodes.size(); i++)
        {
            auto& node = ctrl->nodes[i];
            ImGui::PushID(i);

            ImVec4 c = BlendTypeColor(node.type);

            char label[160];
            std::snprintf(label, sizeof(label), "[%d] %s", i, node.nodeName.c_str());

            ImGui::PushStyleColor(ImGuiCol_Text, c);
            bool selected = (m_SelectedNode == i);
            if (ImGui::Selectable(label, selected))
            {
                m_SelectedNode = i;
                m_SelectedState = -1;
                m_SelectedLink = -1;
                m_RenameTargetNode = -1;
            }
            ImGui::PopStyleColor();

            // Drag source — payload is just the node's index
            if (ImGui::BeginDragDropSource())
            {
                int dragIdx = i;
                ImGui::SetDragDropPayload("BLEND_NODE_IDX", &dragIdx, sizeof(int));
                ImGui::Text("Node %d: %s", i, node.nodeName.c_str());
                ImGui::EndDragDropSource();
            }

            ImGui::Indent();
            switch (node.type)
            {
            case BlendNodeType::Clip:
                ImGui::TextDisabled("%s  %s", ClipName(node.clipID),
                    node.looping ? "(loop)" : "(once)");
                break;
            case BlendNodeType::Blend1D:
                ImGui::TextDisabled("param: %s  (%d entries)",
                    node.parameterName.c_str(), (int)node.blend1DEntries.size());
                break;
            case BlendNodeType::Masked:
                ImGui::TextDisabled("base %d / overlay %d",
                    node.baseNodeIndex, node.overlayNodeIndex);
                break;
            }
            ImGui::Unindent();

            ImGui::Spacing();
            ImGui::PopID();
        }

        ImGui::End();
    }

    // ─── State inspector ─────────────────────────────────────────────────────

    void AnimatorEditorPanel::InspectState(int stateIdx)
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        auto& state = ctrl->states[stateIdx];
        auto& node = ctrl->nodes[state.rootNodeIndex];

        ImGui::TextUnformatted("State");
        ImGui::Separator();

        if (m_RenameTargetState != stateIdx)
        {
            m_RenameTargetState = stateIdx;
            strncpy_s(m_RenameBuffer, state.name.c_str(), sizeof(m_RenameBuffer) - 1);
            m_RenameBuffer[sizeof(m_RenameBuffer) - 1] = '\0';
        }

        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##sname", m_RenameBuffer, sizeof(m_RenameBuffer)))
            state.name = m_RenameBuffer;

        ImGui::Spacing();

        const char* types[] = { "Clip", "Blend 1D", "Masked" };
        int typeIdx = (int)node.type;
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##btype", &typeIdx, types, 3))
            node.type = (BlendNodeType)typeIdx;

        ImGui::Separator();

        switch (node.type)
        {
        case BlendNodeType::Clip:    InspectClipNode(node);    break;
        case BlendNodeType::Blend1D: InspectBlend1DNode(node); break;
        case BlendNodeType::Masked:  InspectMaskedNode(state.rootNodeIndex, node);  break;
        }

        ImGui::Spacing();
        ImGui::Separator();

        ImGui::TextDisabled("Outgoing transitions");
        for (int i = 0; i < (int)ctrl->transitions.size(); i++)
        {
            auto& t = ctrl->transitions[i];
            if (t.fromState != stateIdx && t.fromState != -1) continue;

            const char* toName =
                (t.toState >= 0 && t.toState < (int)ctrl->states.size())
                ? ctrl->states[t.toState].name.c_str() : "???";

            char label[64];
            std::snprintf(label, sizeof(label), "%s → %s (%.2fs)",
                t.fromState == -1 ? "Any" : state.name.c_str(),
                toName, t.duration);

            if (ImGui::Selectable(label, m_SelectedLink == i))
            {
                m_SelectedLink = i;
                m_SelectedState = -1;
            }
        }
    }

    // ─── Transition inspector ─────────────────────────────────────────────────

    void AnimatorEditorPanel::InspectTransition(int transIdx)
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        auto& t = ctrl->transitions[transIdx];

        const char* fromName =
            (t.fromState == -1) ? "Any State"
            : ((t.fromState < (int)ctrl->states.size())
                ? ctrl->states[t.fromState].name.c_str() : "???");

        const char* toName =
            (t.toState >= 0 && t.toState < (int)ctrl->states.size())
            ? ctrl->states[t.toState].name.c_str() : "???";

        ImGui::Text("%s  →  %s", fromName, toName);
        ImGui::Separator();

        ImGui::SetNextItemWidth(-1);
        ImGui::DragFloat("Duration (s)", &t.duration, 0.01f, 0.0f, 5.0f, "%.2f");

        ImGui::Checkbox("Exit Time", &t.hasExitTime);
        if (t.hasExitTime)
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("##et", &t.exitTime, 0.0f, 1.0f, "%.2f");
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Conditions");
        ImGui::Separator();

        std::vector<std::string> paramNames;
        for (auto& [k, v] : ctrl->floatParams) paramNames.push_back(k);
        for (auto& [k, v] : ctrl->boolParams)  paramNames.push_back(k);

        for (int ci = 0; ci < (int)t.conditions.size(); ci++)
        {
            auto& cond = t.conditions[ci];
            ImGui::PushID(ci);

            int curParam = -1;
            for (int pi = 0; pi < (int)paramNames.size(); pi++)
                if (paramNames[pi] == cond.paramName) { curParam = pi; break; }

            std::vector<const char*> names;
            for (auto& s : paramNames) names.push_back(s.c_str());

            ImGui::SetNextItemWidth(90.0f);
            if (ImGui::Combo("##cpname", &curParam, names.data(), (int)names.size()))
            {
                cond.paramName = paramNames[curParam];

                bool newIsBool = ctrl->boolParams.count(cond.paramName) > 0;
                if (newIsBool && !std::holds_alternative<bool>(cond.value))
                {
                    cond.value = false;
                    cond.op = ConditionOp::Equal;
                }
                else if (!newIsBool && !std::holds_alternative<float>(cond.value))
                {
                    cond.value = 0.0f;
                    cond.op = ConditionOp::Greater;
                }
            }

            ImGui::SameLine(0, 4);

            bool isBool = ctrl->boolParams.count(cond.paramName) > 0;

            if (isBool)
            {
                const char* boolOps[] = { "==", "!=" };
                int opIdx = (cond.op == ConditionOp::Equal) ? 0 : 1;
                ImGui::SetNextItemWidth(46.0f);
                if (ImGui::Combo("##cop", &opIdx, boolOps, 2))
                    cond.op = (opIdx == 0) ? ConditionOp::Equal : ConditionOp::NotEqual;

                ImGui::SameLine(0, 4);

                bool bv = std::holds_alternative<bool>(cond.value)
                    ? std::get<bool>(cond.value) : false;
                if (ImGui::Checkbox("##cbv", &bv))
                    cond.value = bv;
            }
            else
            {
                const char* ops[] = { ">", "<", "==", "!=" };
                int opIdx = CondOpIndex(cond.op);
                ImGui::SetNextItemWidth(46.0f);
                if (ImGui::Combo("##cop", &opIdx, ops, 4))
                    cond.op = CondOpFromIndex(opIdx);

                ImGui::SameLine(0, 4);

                float fv = std::holds_alternative<float>(cond.value)
                    ? std::get<float>(cond.value) : 0.0f;
                ImGui::SetNextItemWidth(55.0f);
                if (ImGui::DragFloat("##cfv", &fv, 0.01f))
                    cond.value = fv;
            }

            ImGui::SameLine(0, 6);
            if (ImGui::SmallButton("x"))
            {
                t.conditions.erase(t.conditions.begin() + ci);
                ImGui::PopID();
                break;
            }

            ImGui::PopID();
        }

        if (ImGui::Button("+ Condition") && !paramNames.empty())
        {
            TransitionCondition c;
            c.paramName = paramNames[0];
            bool firstIsBool = ctrl->boolParams.count(c.paramName) > 0;
            if (firstIsBool)
            {
                c.op = ConditionOp::Equal;
                c.value = false;
            }
            else
            {
                c.op = ConditionOp::Greater;
                c.value = 0.0f;
            }
            t.conditions.push_back(c);
        }
    }

    // ─── Blend node inspector ───────────────────────────────────────────────

    void AnimatorEditorPanel::InspectNode(int nodeIdx)
    {
        auto* ctrl = GetController();
        if (!ctrl || nodeIdx < 0 || nodeIdx >= (int)ctrl->nodes.size())
            return;

        auto& node = ctrl->nodes[nodeIdx];

        ImGui::Text("Blend Node [%d]", nodeIdx);
        ImGui::Separator();

        if (m_RenameTargetNode != nodeIdx)
        {
            m_RenameTargetNode = nodeIdx;
            strncpy_s(m_NodeRenameBuffer, node.nodeName.c_str(), sizeof(m_NodeRenameBuffer) - 1);
            m_NodeRenameBuffer[sizeof(m_NodeRenameBuffer) - 1] = '\0';
        }

        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##nodename", m_NodeRenameBuffer, sizeof(m_NodeRenameBuffer)))
            node.nodeName = m_NodeRenameBuffer;

        ImGui::Spacing();

        const char* types[] = { "Clip", "Blend 1D", "Masked" };
        int typeIdx = (int)node.type;
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##ntype", &typeIdx, types, 3))
            node.type = (BlendNodeType)typeIdx;

        ImGui::Separator();

        switch (node.type)
        {
        case BlendNodeType::Clip:    InspectClipNode(node);            break;
        case BlendNodeType::Blend1D: InspectBlend1DNode(node);         break;
        case BlendNodeType::Masked:  InspectMaskedNode(nodeIdx, node); break;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextDisabled("Used by states:");

        bool anyUse = false;
        for (auto& s : ctrl->states)
        {
            if (s.rootNodeIndex == nodeIdx)
            {
                ImGui::BulletText("%s", s.name.c_str());
                anyUse = true;
            }
        }
        if (!anyUse)
            ImGui::TextDisabled("(none \xe2\x80\x94 used as a sub-node only)");
    }


    // ─────────────────────────────────────────────────────────────────────────
    //  Blend-node sub-inspectors
    // ─────────────────────────────────────────────────────────────────────────

    void AnimatorEditorPanel::InspectClipNode(BlendNode& node)
    {
        auto* comp = GetComponent();

        char uuidBuf[64];
        std::snprintf(uuidBuf, sizeof(uuidBuf), "%llu",
            static_cast<unsigned long long>(node.clipID));

        ImGui::TextDisabled("Clip");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##clipid", uuidBuf, sizeof(uuidBuf),
            ImGuiInputTextFlags_ReadOnly);

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* p =
                ImGui::AcceptDragDropPayload("ASSET_ANIMATION"))
            {
                node.clipID = *static_cast<const UUID*>(p->Data);
            }
            ImGui::EndDragDropTarget();
        }

        ImGui::Spacing();

        ImGui::Spacing();

        if (comp && !comp->animationNames.empty())
        {
            ImGui::TextDisabled("Available clips:");
            for (auto& [id, name] : comp->animationNames)
            {
                bool selected = (id == node.clipID);
                if (ImGui::Selectable(name.c_str(), selected))
                    node.clipID = id;
            }
        }

        ImGui::Spacing();
        ImGui::Text("Duration: %.2fs", node.clipDuration);
        ImGui::SameLine();
        ImGui::TextDisabled("(read-only)");

        ImGui::Spacing();
        ImGui::Checkbox("Looping", &node.looping);
        ImGui::DragFloat("Start time", &node.clipTime, 0.01f, 0.0f, 1.0f, "%.2f");
    }

    void AnimatorEditorPanel::InspectBlend1DNode(BlendNode& node)
    {
        auto* comp = GetComponent();
        auto* ctrl = GetController();
        if (!ctrl) return;

        std::vector<std::string> fnStrs;
        for (auto& [k, v] : ctrl->floatParams)
            fnStrs.push_back(k);

        std::vector<const char*> floatNames;
        for (auto& s : fnStrs) floatNames.push_back(s.c_str());

        int curIdx = -1;
        for (int i = 0; i < (int)fnStrs.size(); i++)
            if (fnStrs[i] == node.parameterName) { curIdx = i; break; }

        ImGui::TextDisabled("Drive parameter");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::Combo("##b1dparam", &curIdx, floatNames.data(),
            (int)floatNames.size()) && curIdx >= 0)
            node.parameterName = fnStrs[curIdx];

        ImGui::DragFloat("Playback speed", &node.playbackSpeed, 0.01f, 0.0f, 5.0f, "%.2f");

        ImGui::Spacing();
        ImGui::TextDisabled("Entries  (threshold → clip)");
        ImGui::Separator();

        if (ImGui::BeginTable("##b1d", 3,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableSetupColumn("Clip", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Threshold", ImGuiTableColumnFlags_WidthFixed, 64.0f);
            ImGui::TableSetupColumn("##del", ImGuiTableColumnFlags_WidthFixed, 20.0f);
            ImGui::TableHeadersRow();

            for (int i = 0; i < (int)node.blend1DEntries.size(); i++)
            {
                auto& e = node.blend1DEntries[i];
                ImGui::PushID(i);
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                const char* cname = ClipName(e.animID);
                ImGui::SetNextItemWidth(-1);
                if (ImGui::BeginCombo("##eclip", cname))
                {
                    if (comp)
                    {
                        for (auto& [id, name] : comp->animationNames)
                        {
                            bool sel = (id == e.animID);
                            if (ImGui::Selectable(name.c_str(), sel))
                                e.animID = id;
                        }
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* p =
                        ImGui::AcceptDragDropPayload("ASSET_ANIMATION"))
                        e.animID = *static_cast<const UUID*>(p->Data);
                    ImGui::EndDragDropTarget();
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-1);
                if (ImGui::DragFloat("##et", &e.threshold, 0.01f))
                {
                    std::sort(node.blend1DEntries.begin(), node.blend1DEntries.end(),
                        [](auto& a, auto& b) { return a.threshold < b.threshold; });
                }

                ImGui::TableSetColumnIndex(2);
                if (ImGui::SmallButton("x"))
                {
                    node.blend1DEntries.erase(node.blend1DEntries.begin() + i);
                    ImGui::PopID();
                    ImGui::EndTable();
                    return;
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        if (ImGui::Button("+ Entry"))
            node.AddEntry(UUID::Null, 0.0f);
    }

    void AnimatorEditorPanel::InspectMaskedNode(int nodeIdx, BlendNode& node)
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        auto drawSlot = [&](const char* label, int& target)
            {
                ImGui::PushID(label);
                ImGui::TextDisabled("%s", label);

                char buf[64];
                if (target >= 0 && target < (int)ctrl->nodes.size())
                    std::snprintf(buf, sizeof(buf), "[%d] %s", target, ctrl->nodes[target].nodeName.c_str());
                else
                    std::snprintf(buf, sizeof(buf), "(none)");

                ImGui::Button(buf, ImVec2(-1, 28));

                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("BLEND_NODE_IDX"))
                    {
                        int dropped = *static_cast<const int*>(p->Data);
                        if (dropped == nodeIdx)
                            ImGui::SetTooltip("A masked node cannot reference itself");
                        else
                            target = dropped;
                    }
                    ImGui::EndDragDropTarget();
                }

                int manual = target;
                ImGui::SetNextItemWidth(-1);
                if (ImGui::DragInt("##manual", &manual, 1.0f, -1, (int)ctrl->nodes.size() - 1))
                {
                    if (manual != nodeIdx)
                        target = manual;
                }

                ImGui::PopID();
                ImGui::Spacing();
            };

        drawSlot("Base node", node.baseNodeIndex);
        drawSlot("Overlay node", node.overlayNodeIndex);

        ImGui::Spacing();
        ImGui::TextDisabled("Layer Weight");

        ImGui::SetNextItemWidth(-1);

        float weight = node.weight;
        if (ImGui::SliderFloat("##MaskedWeight", &weight, 0.0f, 1.0f, "%.2f"))
        {
            node.weight = weight;
        }

        if (node.baseNodeIndex == nodeIdx || node.overlayNodeIndex == nodeIdx)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.35f, 0.35f, 1.0f));
            ImGui::TextWrapped("Warning: this node references itself \xe2\x80\x94 fix base/overlay.");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::TextDisabled("Bone mask");

        std::string buttonText = "Drop bone mask here##bmd";

        auto boneMask = assetManager.GetBoneMask(node.boneMaskID);

        if (boneMask) 
        {
            buttonText = boneMask->name + "##bmd";

            // TODO : add refresh boneMask button 
            node.boneMask = boneMask->boneMask;
        }

        ImGui::Button(buttonText.c_str(), ImVec2(-1, 28));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* p =
                ImGui::AcceptDragDropPayload("BONE_MASK_ASSET"))
            {
                const auto* payload =
                    static_cast<const BoneMaskDragPayload*>(p->Data);

                UUID boneMaskId = payload->id;
                node.boneMaskID = boneMaskId;

                auto droppedBoneMask = assetManager.GetBoneMask(boneMaskId);
                node.boneMask = droppedBoneMask->boneMask;

              
            }
            ImGui::EndDragDropTarget();
        }

        if (!node.boneMask.empty())
        {
            ImGui::Spacing();
            ImGui::TextDisabled("%d bone weights", (int)node.boneMask.size());

            const float barH = 4.0f;
            const float spacing = 2.0f;
            const int   maxShow = 32;

            int show = std::min((int)node.boneMask.size(), maxShow);
            for (int i = 0; i < show; i++)
            {
                ImGui::PushID(i);
                ImGui::ProgressBar(node.boneMask[i], ImVec2(-1, barH), "");
                ImGui::PopID();
                ImGui::Dummy(ImVec2(0, spacing));
            }
            if ((int)node.boneMask.size() > maxShow)
                ImGui::TextDisabled("... +%d more",
                    (int)node.boneMask.size() - maxShow);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Helpers
    // ─────────────────────────────────────────────────────────────────────────

    const char* AnimatorEditorPanel::ClipName(UUID id) const
    {
        auto* comp = GetComponent();
        if (!comp) return "(no component)";
        auto it = comp->animationNames.find(id);
        return (it != comp->animationNames.end())
            ? it->second.c_str()
            : "(unknown)";
    }

    const char* AnimatorEditorPanel::BlendTypeLabel(BlendNodeType t) const
    {
        switch (t)
        {
        case BlendNodeType::Clip:    return "Clip";
        case BlendNodeType::Blend1D: return "1D Blend";
        case BlendNodeType::Masked:  return "Masked";
        }
        return "?";
    }

    ImVec4 AnimatorEditorPanel::BlendTypeColor(BlendNodeType t) const
    {
        switch (t)
        {
        case BlendNodeType::Clip:    return { 0.21f, 0.62f, 0.46f, 1.f };
        case BlendNodeType::Blend1D: return { 0.21f, 0.55f, 0.80f, 1.f };
        case BlendNodeType::Masked:  return { 0.56f, 0.35f, 0.71f, 1.f };
        }
        return { 0.4f, 0.4f, 0.4f, 1.f };
    }

    const char* AnimatorEditorPanel::CondOpLabel(ConditionOp op) const
    {
        switch (op)
        {
        case ConditionOp::Greater:  return ">";
        case ConditionOp::Less:     return "<";
        case ConditionOp::Equal:    return "==";
        case ConditionOp::NotEqual: return "!=";
        }
        return "?";
    }

    int AnimatorEditorPanel::CondOpIndex(ConditionOp op) const
    {
        switch (op)
        {
        case ConditionOp::Greater:  return 0;
        case ConditionOp::Less:     return 1;
        case ConditionOp::Equal:    return 2;
        case ConditionOp::NotEqual: return 3;
        }
        return 0;
    }

    ConditionOp AnimatorEditorPanel::CondOpFromIndex(int idx) const
    {
        switch (idx)
        {
        case 0: return ConditionOp::Greater;
        case 1: return ConditionOp::Less;
        case 2: return ConditionOp::Equal;
        case 3: return ConditionOp::NotEqual;
        }
        return ConditionOp::Greater;
    }

    void AnimatorEditorPanel::AddDefaultState()
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        int nodeIdx = ctrl->AddNode(BlendNode::MakeClip(UUID::Null, true));

        char name[32];
        std::snprintf(name, sizeof(name), "State %d", (int)ctrl->states.size());

        int stateIdx = ctrl->AddState(name, nodeIdx);

        if (ctrl->currentState < 0)
            ctrl->SetDefaultState(stateIdx);

        ImNodes::EditorContextSet(m_NodesCtx);
        ImNodes::SetNodeGridSpacePos(stateIdx,
            ImVec2(200.0f + stateIdx * 20.0f, 200.0f + stateIdx * 20.0f));

        m_SelectedState = stateIdx;
    }

    void AnimatorEditorPanel::DeleteSelectedNodes()
    {
        auto* ctrl = GetController();
        if (!ctrl || m_SelectedState < 0) return;

        int idx = m_SelectedState;

        ctrl->transitions.erase(
            std::remove_if(ctrl->transitions.begin(), ctrl->transitions.end(),
                [idx](const AnimTransition& t)
                {
                    return t.fromState == idx || t.toState == idx;
                }),
            ctrl->transitions.end());
        int nodeIdx = ctrl->states[idx].rootNodeIndex;
        ctrl->nodes.erase(ctrl->nodes.begin() + nodeIdx);

        for (auto& s : ctrl->states)
            if (s.rootNodeIndex > nodeIdx)
                s.rootNodeIndex--;

        for (auto& n : ctrl->nodes)
        {
            if (n.type != BlendNodeType::Masked) continue;

            if (n.baseNodeIndex == nodeIdx)      n.baseNodeIndex = -1;
            else if (n.baseNodeIndex > nodeIdx)  n.baseNodeIndex--;

            if (n.overlayNodeIndex == nodeIdx)     n.overlayNodeIndex = -1;
            else if (n.overlayNodeIndex > nodeIdx) n.overlayNodeIndex--;
        }

        ctrl->states.erase(ctrl->states.begin() + idx);

        for (auto& t : ctrl->transitions)
        {
            if (t.fromState > idx) t.fromState--;
            if (t.toState > idx) t.toState--;
        }

        if (ctrl->currentState == idx)
            ctrl->currentState = ctrl->states.empty() ? -1 : 0;
        else if (ctrl->currentState > idx)
            ctrl->currentState--;

        if (m_SelectedNode == nodeIdx)      m_SelectedNode = -1;
        else if (m_SelectedNode > nodeIdx)  m_SelectedNode--;

        m_SelectedState = -1;
    }

    void AnimatorEditorPanel::DeleteSelectedLinks()
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        if (m_SelectedLink < 0 ||
            m_SelectedLink >= (int)ctrl->transitions.size())
            return;

        ctrl->transitions.erase(ctrl->transitions.begin() + m_SelectedLink);
        m_SelectedLink = -1;
    }

    void AnimatorEditorPanel::InitNodePositions()
    {
        auto* ctrl = GetController();
        if (!ctrl) return;

        const float startX = 60.0f;
        const float startY = 80.0f;
        const float stepX = 220.0f;
        const float stepY = 160.0f;
        const int   perRow = 4;

        ImNodes::EditorContextSet(m_NodesCtx);
        ImNodes::SetNodeGridSpacePos(k_AnyStateNodeID, ImVec2(startX, startY));

        for (int i = 0; i < (int)ctrl->states.size(); i++)
        {
            float x = startX + ((i % perRow) + 1) * stepX;
            float y = startY + (i / perRow) * stepY;
            ImNodes::SetNodeGridSpacePos(i, ImVec2(x, y));
        }
    }

} // namespace Lengine