#pragma once

#include <queue>
#include <imgui.h>

// Editor
#include "Editor/external/imnodes.h"
#include "Editor/panels/DragDropPayloads.h"


// Engine
#include "resources/AssetManager.h"

#include <string>
#include <unordered_map>

namespace Lengine
{
    // ─── Pin ID encoding ─────────────────────────────────────────────────────
    // We pack (nodeIndex, pinType) into a single int for imnodes.
    // Input pin  = nodeIndex * 2 + 0
    // Output pin = nodeIndex * 2 + 1
    // "Any State" sentinel node lives at index 9999.

    static constexpr int k_AnyStateNodeID = 9999;
    static constexpr int k_AnyStateOutPin = k_AnyStateNodeID * 2 + 1;

    inline int InputPin(int nodeIdx) { return nodeIdx * 2 + 0; }
    inline int OutputPin(int nodeIdx) { return nodeIdx * 2 + 1; }
    inline int NodeFromOutputPin(int pin) { return (pin - 1) / 2; }
    inline int NodeFromInputPin(int pin) { return  pin / 2; }


    // ─── AnimatorEditorPanel ─────────────────────────────────────────────────

    class AnimatorEditorPanel
    {
    public:
        explicit AnimatorEditorPanel(Registry& reg, AssetManager& assets, std::string title = "Animator Editor");

        AnimatorEditorPanel();
        ~AnimatorEditorPanel();

        // Bind to an entity that has an AnimationComponent.  Pass NullEntity to clear.
        void SetTarget(Entity entity);

        // Call once per frame inside your editor's OnImGuiRender().
        void OnImGuiRender();

        bool IsOpen() const { return m_Open; }
        void Close() { m_Open = false; }

    private:
        // ── Registry accessors ───────────────────────────────────────────────
        // These replace the old raw pointer members; always go through the
        // registry so the panel stays valid even if the component is removed.
        AnimationComponent* GetComponent() const;
        AnimatorController* GetController()  const;

        // ── Sub-panels ───────────────────────────────────────────────────────
        void RenderToolbar();
        void RenderParameterSidebar();
        void RenderGraphCanvas();
        void RenderInspectorSidebar();
        void RenderBlendNodesPanel();

        // ── Node drawing ─────────────────────────────────────────────────────
        void DrawStateNode(int stateIdx);
        void DrawAnyStateNode();
        void DrawLinks();

        // ── Inspector contents ───────────────────────────────────────────────
        void InspectState(int stateIdx);
        void InspectTransition(int transIdx);
        void InspectNode(int nodeIdx);

        // ── Blend-node sub-inspectors ────────────────────────────────────────
        void InspectClipNode(BlendNode& node);
        void InspectBlend1DNode(BlendNode& node);
        void InspectMaskedNode(int nodeIdx, BlendNode& node);

        // ── Helpers ──────────────────────────────────────────────────────────
        const char* ClipName(UUID id)           const;
        const char* BlendTypeLabel(BlendNodeType t) const;
        ImVec4      BlendTypeColor(BlendNodeType t) const;
        const char* CondOpLabel(ConditionOp op)     const;
        int         CondOpIndex(ConditionOp op)     const;
        ConditionOp CondOpFromIndex(int idx)        const;

        void AddDefaultState();
        void DeleteSelectedNodes();
        void DeleteSelectedLinks();
        void InitNodePositions();

    private:
        Registry& registry;
        AssetManager& assetManager;
        // Target entity — component access goes through registry.animations
        // rather than a raw pointer that can dangle.
        Entity m_Entity = NullEntity;

        std::string m_Title;

        ImNodesEditorContext* m_NodesCtx = nullptr;

        bool m_Open = true;
        bool m_NeedsLayout = true;

        // Selection state
        int m_SelectedState = -1;
        int m_SelectedLink = -1;
        int m_SelectedNode = -1;

        // Rename buffer
        char m_RenameBuffer[128] = {};
        int  m_RenameTargetState = -1;
        char m_NodeRenameBuffer[128] = {};           
        int  m_RenameTargetNode = -1;                

        bool m_ShowBlendNodesPanel = true;           

        // Parameter-add buffers
        char m_NewParamName[64] = {};
        int  m_NewParamType = 0;   // 0 = float, 1 = bool

        std::unordered_map<int, ImVec2> m_NodePositions;

    private:
        // delete in queue
        std::queue<std::string> floatParamsToBeDeleted;
        std::queue<std::string> boolParamsToBeDeleted;

    };

} // namespace Lengine