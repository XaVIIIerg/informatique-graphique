// Projet Info Graphique - Court-métrage final
// Pour lancer le court-métrage, il suffit de lancer le fichier project.cpp car tout est en une scène.

#include <Viewer.hpp>
#include <ShaderProgram.hpp>
#include <vector>

#include <texturing/TexturedPlaneRenderable.hpp>
#include <texturing/TexturedCubeRenderable.hpp>
#include <texturing/TexturedMeshRenderable.hpp>
#include <texturing/TexturedLightedMeshRenderable.hpp>
#include <texturing/MultiTexturedCubeRenderable.hpp>
#include <texturing/MipMapCubeRenderable.hpp>
#include <texturing/BillBoardPlaneRenderable.hpp>
#include <lighting/DirectionalLightRenderable.hpp>
#include <texturing/TexturedTriangleRenderable.hpp>
#include <texturing/CubeMapRenderable.hpp>
#include <FrameRenderable.hpp>
#include <Camera.hpp>
#include <iostream>
#include <lighting/LightedMeshRenderable.hpp>
#include <lighting/LightedCubeRenderable.hpp>
#include <lighting/LightedCylinderRenderable.hpp>
#include <lighting/DirectionalLightRenderable.hpp>
#include <lighting/PointLightRenderable.hpp>
#include <lighting/SpotLightRenderable.hpp>
#include <texturing/EnvMapMeshRenderable.hpp>

#include <QuadMeshRenderable.hpp>
#include <FrameRenderable.hpp>

#include <dynamics/DynamicSystemRenderable.hpp>
#include <dynamics/DampingForceField.hpp>
#include <dynamics/ConstantForceField.hpp>
#include <dynamics/SpringForceField.hpp>
#include <dynamics/EulerExplicitSolver.hpp>

#include <dynamics/ParticleRenderable.hpp>
#include <dynamics/ParticleListRenderable.hpp>
#include <dynamics/ConstantForceFieldRenderable.hpp>
#include <dynamics/SpringForceFieldRenderable.hpp>
#include <dynamics/SpringListRenderable.hpp>
#include <dynamics/ControlledForceFieldRenderable.hpp>
#include <cstdlib>
#include <ctime>
#include <random>
#include <algorithm>

// Structures pour les pingouins
// PenguinParts : contient les parties du modèle (tronc, pattes, ailes)
// Estrade : paramètres de la position et de la rotation nécessaire pour placer une rangée de pingouins
struct PenguinParts
{
    TexturedLightedMeshRenderablePtr tronc;
    TexturedLightedMeshRenderablePtr piedDroit;
    TexturedLightedMeshRenderablePtr piedGauche;
    TexturedLightedMeshRenderablePtr aileDroite;
    TexturedLightedMeshRenderablePtr aileGauche;
};

struct Estrade
{
    float zPosition; // Position en Z 
    float yOffset;   // Décalage en Y 
    float xOffset;   // Décalage en X (0.5 pour l'estrade du milieu)
    float rotation;  // Rotation des pingouins (-M_PI/2 ou M_PI/2)
    int index;       // Index de l'estrade (0-5)
};

// Variables globales pour les pingouins
std::vector<TexturedLightedMeshRenderablePtr> tousLesPingouins;
std::vector<std::vector<TexturedLightedMeshRenderablePtr>> pingouinsParEstrade(6);

// Fonction pour appliquer l'animation à un pingouin
// définit les keyframes des ailes et du tronc
// ajoute des keyframes répétitives jusqu'à des temps définis (7s, 66s, 100s)
// Paramètres : 
// - pingouin : structure contenant les parties du pingouin à animer
// - rotationY : angle de rotation autour de l'axe Y (orientation du modèle)
void appliquerAnimation(PenguinParts &pingouin, float rotationY)
{
    // Récupérer la transformation globale actuelle
    glm::mat4 baseTransform = pingouin.tronc->getGlobalTransform();

    // Appliquer les transformations locales des ailes
    pingouin.aileGauche->setLocalTransform(
        getRotationMatrix(-1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(-0.28f, -0.05f, 0.0f));

    pingouin.aileDroite->setLocalTransform(
        getRotationMatrix(1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(0.28f, -0.05f, 0.0f));


    // Position finale de l'aile droite de 0 secondes à 75 secondes (transformation de base)
    glm::mat4 aileDroiteFinal = getRotationMatrix(1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                getTranslationMatrix(0.28f, -0.05f, 0.0f);

    pingouin.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 0);

    // Animation de WAVE énergique avec l'aile droite - boucle jusqu'à 7 secondes
    float waveCycle = 0.4f; // durée d'un cycle de wave
    int numWaves = (int)(7.0f / waveCycle); // nombre de cycles dans 7 secondes

    for (int i = 0; i <= numWaves; i++)
    {
        float startTime = i * waveCycle; // temps de début du cycle
        if (startTime <= 7.0f)
        {
            // Keyframes pour l'aile gauche : position neutre, relevée, puis neutre
            pingouin.aileGauche->addLocalTransformKeyframe(GeometricTransformation(), startTime);
            pingouin.aileGauche->addLocalTransformKeyframe(
                GeometricTransformation(
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::angleAxis(glm::radians(10.0f), glm::vec3(0, 0, 1)),
                    glm::vec3(1)),
                startTime + 0.2f);
            pingouin.aileGauche->addLocalTransformKeyframe(GeometricTransformation(), startTime + 0.4f);
        }
    }

    // Tronc : animation GLOBALE qui s'ajoute à la transformation de base - boucle jusqu'à 7 secondes
    // Créer les transformations d'animation
    glm::mat4 anim0 = getTranslationMatrix(0.0f, 0.0f, 0.0f) *
                      getRotationMatrix(glm::radians(-6.0f), glm::vec3(0, 0, 1)); // position neutre

    glm::mat4 anim1 = getTranslationMatrix(0.0f, 0.03f, 0.0f) *
                      getRotationMatrix(glm::radians(8.0f), glm::vec3(0, 0, 1)); // inclinaison droite

    glm::mat4 anim2 = getTranslationMatrix(0.0f, 0.0f, 0.0f) *
                      getRotationMatrix(glm::radians(-6.0f), glm::vec3(0, 0, 1)); // retour position neutre

    // Appliquer: baseTransform x animation - boucle chaque seconde
    float swingCycle = 1.0f; // durée d'un cycle de swing
    int numSwings = (int)(7.0f / swingCycle); // nombre de cycles dans 7 secondes

    for (int i = 0; i <= numSwings; i++)
    {
        float startTime = i * swingCycle;
        if (startTime <= 7.0f)
        {
            // Keyframes pour le tronc : appliquer les transformations d'animation
            pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(baseTransform * anim0), startTime);

            if (startTime + 0.5f <= 7.0f)
            {
                pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(baseTransform * anim1), startTime + 0.5f);
            }

            if (startTime + 1.0f <= 7.0f)
            {
                pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(baseTransform * anim2), startTime + 1.0f);
            }
        }
    }

    // À 7 secondes: position finale avec échelle (2,2,2)
    glm::mat4 finalTransform = baseTransform;
    pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(finalTransform), 8.0f);

    // À 66 secondes: même position (pour rester figé)
    pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(finalTransform), 66.0f);

    // Position finale des ailes à 7 secondes (transformations de base)
    glm::mat4 aileGaucheFinal = getRotationMatrix(-1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                getTranslationMatrix(-0.28f, -0.05f, 0.0f);

    pingouin.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 8.0f);

    // À 100 secondes: mêmes positions
    pingouin.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 66.0f);
    pingouin.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 100.0f);

    // Recommencer l'animation après 66 secondes jusqu'à 100 secondes
    // Animation de WAVE énergique avec l'aile droite - boucle jusqu'à 100 secondes
    waveCycle = 0.4f; // durée d'un cycle de wave
    numWaves = (int)(40.0f / waveCycle);

    for (int i = 0; i <= numWaves; i++)
    {
        float startTime = i * waveCycle + 66.0f;
        if (startTime <= 7.0f)
        {
            pingouin.aileGauche->addLocalTransformKeyframe(GeometricTransformation(), startTime);
            pingouin.aileGauche->addLocalTransformKeyframe(
                GeometricTransformation(
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::angleAxis(glm::radians(10.0f), glm::vec3(0, 0, 1)),
                    glm::vec3(1)),
                startTime + 0.2f);
            pingouin.aileGauche->addLocalTransformKeyframe(GeometricTransformation(), startTime + 0.4f);
        }
    }

    // Tronc : animation GLOBALE qui s'ajoute à la transformation de base - boucle jusqu'à 100 secondes

    // Appliquer: baseTransform x animation - boucle chaque seconde
    swingCycle = 1.0f;
    numSwings = (int)(40.0f / swingCycle);

    for (int i = 0; i <= numSwings; i++)
    {
        float startTime = i * swingCycle + 66.0f;
        if (startTime <= 7.0f)
        {
            pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(baseTransform * anim0), startTime);

            if (startTime + 0.5f <= 7.0f)
            {
                pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(baseTransform * anim1), startTime + 0.5f);
            }

            if (startTime + 1.0f <= 7.0f)
            {
                pingouin.tronc->addGlobalTransformKeyframe(GeometricTransformation(baseTransform * anim2), startTime + 1.0f);
            }
        }
    }
}

// Fonction pour créer un pingouin
PenguinParts createPenguin(ShaderProgramPtr texShader, MaterialPtr pingouinMaterial,
                           float rotationY, float posX, float posY, float posZ,
                           float scale = 2.0f)
{
    PenguinParts parts;

    // Charger les meshes
    parts.piedDroit = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_patedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/private2.png");

    parts.piedGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_pategauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/private2.png");

    parts.aileDroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/private2.png");

    parts.aileGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailegauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/private2.png");

    parts.tronc = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_tronc.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/private2.png");

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(parts.tronc, parts.piedDroit);
    HierarchicalRenderable::addChild(parts.tronc, parts.piedGauche);
    HierarchicalRenderable::addChild(parts.tronc, parts.aileDroite);
    HierarchicalRenderable::addChild(parts.tronc, parts.aileGauche);

    // Appliquer la transformation
    glm::mat4 transform = getRotationMatrix(rotationY, 0, 1, 0) *
                          getTranslationMatrix(posX, posY, posZ) *
                          getScaleMatrix(scale);
    parts.tronc->setGlobalTransform(transform);

    return parts;
}

// Crée toutes les rangées de pingouins et les ajoute au viewer
// Deux jeux d'estrades (gauche/droite) sont définis avec des offsets et rotations.
// Pour chaque estrade, on parcourt une ligne en X de startX à endX avec un espacement fixe.
// Chaque pingouin est créé via createPenguin animé avec appliquerAnimation et ajouté au viewer.
// Les pingouins sont stockés globalement dans tousLesPingouins et pingouinsParEstrade.
// De base, c'était pour pouvoir les animer tous ensemble ou par estrade plus tard mais j"ai jamais utilisé cette fonctionnalité
// Paramètres :
// - viewer : instance du Viewer pour ajouter les renderables
// - texShader : shader program pour les pingouins
// - pingouinMaterial : matériau pour les pingouins
void createAllPenguins(Viewer &viewer, ShaderProgramPtr texShader, MaterialPtr pingouinMaterial)
{
    // Configuration des estrades
    std::vector<Estrade> estradesGauche = {
        {-4.2f, -1.2f, 0.0f, -M_PI / 2, 0},
        {-3.2f, -1.3f, 0.5f, -M_PI / 2, 1},
        {-2.3f, -1.4f, 0.0f, -M_PI / 2, 2}};

    std::vector<Estrade> estradesDroite = {
        {-4.2f, -1.2f, 0.0f, M_PI / 2, 3},
        {-3.2f, -1.3f, 0.5f, M_PI / 2, 4},
        {-2.3f, -1.4f, 0.0f, M_PI / 2, 5}};

    // Paramètres pour la disposition des pingouins
    float startX = 5.5f; // Position de départ en X
    float endX = -5.5f; // Position de fin en X
    float spacing = 1.0f; // Espacement entre les pingouins

    // Créer les pingouins côté gauche
    for (const auto &estrade : estradesGauche)
    {
        for (float x = startX; x >= endX; x -= spacing)
        {
            // Créer un pingouin à la position calculée
            PenguinParts pingouin = createPenguin(texShader, pingouinMaterial,
                                                  estrade.rotation, x + estrade.xOffset, estrade.yOffset, estrade.zPosition);

            // Appliquer l'animation
            appliquerAnimation(pingouin, estrade.rotation);
            viewer.addRenderable(pingouin.tronc);

            tousLesPingouins.push_back(pingouin.tronc);
            pingouinsParEstrade[estrade.index].push_back(pingouin.tronc);
        }
    }

    // Créer les pingouins côté droit
    for (const auto &estrade : estradesDroite)
    {
        for (float x = startX; x >= endX; x -= spacing)
        {
            PenguinParts pingouin = createPenguin(texShader, pingouinMaterial,
                                                  estrade.rotation, x + estrade.xOffset, estrade.yOffset, estrade.zPosition);

            appliquerAnimation(pingouin, estrade.rotation);
            viewer.addRenderable(pingouin.tronc);

            tousLesPingouins.push_back(pingouin.tronc);
            pingouinsParEstrade[estrade.index].push_back(pingouin.tronc);
        }
    }
}

// Fonction pour initialiser le jury
void initJury(Viewer &viewer, ShaderProgramPtr texShader, MaterialPtr pingouinMaterial)
{
    // ------ PINGOUIN 1 JURY ROSE -------
    PenguinParts parts1;

    // Charger les meshes
    parts1.piedDroit = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_patedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinrose.png");

    parts1.piedGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_pategauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinrose.png");

    parts1.aileDroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinrose.png");

    parts1.aileGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailegauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinrose.png");

    parts1.tronc = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_tronc.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinrose.png");

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(parts1.tronc, parts1.piedDroit);
    HierarchicalRenderable::addChild(parts1.tronc, parts1.piedGauche);
    HierarchicalRenderable::addChild(parts1.tronc, parts1.aileDroite);
    HierarchicalRenderable::addChild(parts1.tronc, parts1.aileGauche);

    // Appliquer la transformation
    glm::mat4 transform = getRotationMatrix(M_PI, 0, 1, 0) *
                          getTranslationMatrix(0.0, -1.2, -5.5) *
                          getScaleMatrix(2.5);
    // Le pingouin est debout au début
    parts1.tronc->addGlobalTransformKeyframe(transform, 0.0);
    parts1.tronc->addGlobalTransformKeyframe(transform, 62.0);
    // Le pingouin tombe à 63 secondes puis reste à terre
    parts1.tronc->addGlobalTransformKeyframe(transform * getRotationMatrix(-M_PI / 2, 1, 0, 0), 63.0);
    parts1.tronc->addGlobalTransformKeyframe(transform * getRotationMatrix(-M_PI / 2, 1, 0, 0), 100.0);

    parts1.aileGauche->setLocalTransform(
        getRotationMatrix(-1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(-0.28f, -0.05f, 0.0f));

    parts1.aileDroite->setLocalTransform(
        getRotationMatrix(1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(0.28f, -0.05f, 0.0f));

    viewer.addRenderable(parts1.tronc);

    // -------- PINGOUIN 2 JURY BLEU --------
    PenguinParts parts2;

    // Charger les meshes
    parts2.piedDroit = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_patedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinbleu.png");

    parts2.piedGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_pategauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinbleu.png");

    parts2.aileDroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinbleu.png");

    parts2.aileGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailegauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinbleu.png");

    parts2.tronc = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_tronc.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinbleu.png");

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(parts2.tronc, parts2.piedDroit);
    HierarchicalRenderable::addChild(parts2.tronc, parts2.piedGauche);
    HierarchicalRenderable::addChild(parts2.tronc, parts2.aileDroite);
    HierarchicalRenderable::addChild(parts2.tronc, parts2.aileGauche);

    // Appliquer la transformation
    transform = getRotationMatrix(M_PI, 0, 1, 0) *
                getTranslationMatrix(1.0, -1.2, -5.5) *
                getScaleMatrix(2.5);
    parts2.tronc->setGlobalTransform(transform);

    // Le pingouin est debout au début
    parts2.tronc->addGlobalTransformKeyframe(transform, 0.0);
    parts2.tronc->addGlobalTransformKeyframe(transform, 62.0);
    // Le pingouin tombe à 63 secondes puis reste à terre
    parts2.tronc->addGlobalTransformKeyframe(transform * getRotationMatrix(-M_PI / 2, 1, 0, 0), 63.0);
    parts2.tronc->addGlobalTransformKeyframe(transform * getRotationMatrix(-M_PI / 2, 1, 0, 0), 100.0);

    parts2.aileGauche->setLocalTransform(
        getRotationMatrix(-1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(-0.28f, -0.05f, 0.0f));

    parts2.aileDroite->setLocalTransform(
        getRotationMatrix(1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(0.28f, -0.05f, 0.0f));

    viewer.addRenderable(parts2.tronc);

    // ------- PINGOUIN 3 JURY VERT -------
    PenguinParts parts3;

    // Charger les meshes
    parts3.piedDroit = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_patedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinvert.png");

    parts3.piedGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_pategauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinvert.png");

    parts3.aileDroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailedroite.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinvert.png");

    parts3.aileGauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_ailegauche.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinvert.png");

    parts3.tronc = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/pingouin2/vraipingouin_tronc.obj",
        pingouinMaterial, "../../sfmlGraphicsPipeline/textures/pingouinvert.png");

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(parts3.tronc, parts3.piedDroit);
    HierarchicalRenderable::addChild(parts3.tronc, parts3.piedGauche);
    HierarchicalRenderable::addChild(parts3.tronc, parts3.aileDroite);
    HierarchicalRenderable::addChild(parts3.tronc, parts3.aileGauche);

    // Appliquer la transformation
    transform = getRotationMatrix(M_PI, 0, 1, 0) *
                getTranslationMatrix(-1.0, -1.2, -5.5) *
                getScaleMatrix(2.5);
    parts3.tronc->setGlobalTransform(transform);

    // Le pingouin est debout au début
    parts3.tronc->addGlobalTransformKeyframe(transform, 0.0);
    parts3.tronc->addGlobalTransformKeyframe(transform, 62.0);
    // Le pingouin tombe à 63 secondes puis reste à terre
    parts3.tronc->addGlobalTransformKeyframe(transform * getRotationMatrix(-M_PI / 2, 1, 0, 0), 63.0);
    parts3.tronc->addGlobalTransformKeyframe(transform * getRotationMatrix(-M_PI / 2, 1, 0, 0), 100.0);

    parts3.aileGauche->setLocalTransform(
        getRotationMatrix(-1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(-0.28f, -0.05f, 0.0f));

    parts3.aileDroite->setLocalTransform(
        getRotationMatrix(1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
        getTranslationMatrix(0.28f, -0.05f, 0.0f));

    viewer.addRenderable(parts3.tronc);

    // Création des différentes pancartes
    const std::string panneau = "../../sfmlGraphicsPipeline/meshes/panneau/panneaufinal.obj";
    const std::string p0_path = "../../sfmlGraphicsPipeline/textures/panneau/wood00.jpg";
    const std::string p1_path = "../../sfmlGraphicsPipeline/textures/panneau/wood1.jpg";
    const std::string p2_path = "../../sfmlGraphicsPipeline/textures/panneau/wood2.jpg";
    const std::string p7_path = "../../sfmlGraphicsPipeline/textures/panneau/wood7.jpg";
    const std::string p8_path = "../../sfmlGraphicsPipeline/textures/panneau/wood8.jpg";
    const std::string p10_path = "../../sfmlGraphicsPipeline/textures/panneau/wood10.jpg";

    auto p0_1 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p0_path);
    auto p0_2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p0_path);
    auto p1_1 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p1_path);
    auto p1_2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p1_path);
    auto p2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p2_path);
    auto p7 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p7_path);
    auto p8_1 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p8_path);
    auto p8_2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p8_path);
    auto p10 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, panneau,
        pingouinMaterial, p10_path);

    // Attacher les pancartes aux ailes des juges
    HierarchicalRenderable::addChild(parts1.aileGauche, p7);
    HierarchicalRenderable::addChild(parts2.aileGauche, p8_1);
    HierarchicalRenderable::addChild(parts3.aileGauche, p8_2);
    HierarchicalRenderable::addChild(parts1.aileGauche, p1_1);
    HierarchicalRenderable::addChild(parts2.aileGauche, p1_2);
    HierarchicalRenderable::addChild(parts3.aileGauche, p2);

    // Animation des ailes des juges :
    // position finale des ailes
    glm::mat4 aileGaucheFinal = getRotationMatrix(-1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                getTranslationMatrix(-0.28f, -0.05f, 0.0f);
    glm::mat4 aileDroiteFinal = getRotationMatrix(1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                getTranslationMatrix(0.28f, -0.05f, 0.0f);
    // position des ailes avec panneau
    glm::mat4 aileGauchePanneau = getRotationMatrix(1.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
                                  getTranslationMatrix(0.2f, -0.21f, 0.0f);

    // Définir les keyframes pour les ailes des trois pingouins du jury : à la première seconde, elles sont baissées et elles le restent jusqu'à la seconde 21
    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 0.0f);
    parts1.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 0.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 0.0f);
    parts2.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 0.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 0.0f);
    parts3.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 0.0f);

    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 21.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 21.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 21.0f);

    // Les ailes gauches se lèvent entre la seconde 22 et 29, puis redescendent et remontent entre 31 et 47
    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 22.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 22.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 22.0f);

    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 29.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 29.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 29.0f);

    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 30.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 30.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 30.0f);

    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 31.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 31.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 31.0f);

    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 45.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 45.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGauchePanneau), 45.0f);

    // Les ailes restent baissées jusqu'à la fin de l'animation
    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 47.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 47.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 47.0f);

    parts1.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 500.0f);
    parts1.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 500.0f);
    parts2.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 500.0f);
    parts2.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 500.0f);
    parts3.aileGauche->addLocalTransformKeyframe(GeometricTransformation(aileGaucheFinal), 500.0f);
    parts3.aileDroite->addLocalTransformKeyframe(GeometricTransformation(aileDroiteFinal), 500.0f);

    // Animation des pancartes des juges :
    // position des pancartes dans la main
    glm::mat4 panneauOffset =
        getTranslationMatrix(0.18f, 0.45f, 0.02f) * // réglage position main
        getScaleMatrix(0.05f);
    glm::mat4 panneauadios = getTranslationMatrix(4.0f, 4.0f, 4.0f) * // le mettre en dehors de la pièce
                             getScaleMatrix(0.05f);

    // Définir les keyframes pour les premières pancartes des trois pingouins du jury : elles apparaissent à la seconde 22 et restent jusqu'à la seconde 29
    p7->addLocalTransformKeyframe(panneauOffset, 22.0f);
    p8_1->addLocalTransformKeyframe(panneauOffset, 22.0f);
    p8_2->addLocalTransformKeyframe(panneauOffset, 22.0f);

    p7->addLocalTransformKeyframe(panneauOffset, 29.0f);
    p8_1->addLocalTransformKeyframe(panneauOffset, 29.0f);
    p8_2->addLocalTransformKeyframe(panneauOffset, 29.0f);

    // Les pancartes disparaissent à la seconde 30 et sont placées hors de la pièce
    p7->addLocalTransformKeyframe(panneauadios, 30.0f);
    p8_1->addLocalTransformKeyframe(panneauadios, 30.0f);
    p8_2->addLocalTransformKeyframe(panneauadios, 30.0f);

    p7->addLocalTransformKeyframe(panneauadios, 500.0f);
    p8_1->addLocalTransformKeyframe(panneauadios, 500.0f);
    p8_2->addLocalTransformKeyframe(panneauadios, 500.0f);

    // Définir les keyframes pour les deuxièmes pancartes des trois pingouins du jury : elles apparaissent à la seconde 31 et restent jusqu'à la seconde 45
    p2->addLocalTransformKeyframe(panneauOffset, 31.0f);
    p1_1->addLocalTransformKeyframe(panneauOffset, 31.0f);
    p1_2->addLocalTransformKeyframe(panneauOffset, 31.0f);

    p2->addLocalTransformKeyframe(panneauOffset, 45.0f);
    p1_1->addLocalTransformKeyframe(panneauOffset, 45.0f);
    p1_2->addLocalTransformKeyframe(panneauOffset, 45.0f);

    // Les pancartes disparaissent à la seconde 47 et sont placées hors de la pièce
    p2->addLocalTransformKeyframe(panneauadios, 47.0f);
    p1_1->addLocalTransformKeyframe(panneauadios, 47.0f);
    p1_2->addLocalTransformKeyframe(panneauadios, 47.0f);

    p2->addLocalTransformKeyframe(panneauadios, 500.0f);
    p1_1->addLocalTransformKeyframe(panneauadios, 500.0f);
    p1_2->addLocalTransformKeyframe(panneauadios, 500.0f);

    // Animation de la troisième série de pancartes : elles apparaîssent à la seconde 56 sur le banc des jurys et reste jusqu'à la fin
    p10->addGlobalTransformKeyframe(GeometricTransformation(getRotationMatrix(-M_PI / 2, 1, 0, 0) * getRotationMatrix(-M_PI, 0, 0, 1) *
                                                            getTranslationMatrix(-0.5, 5.0, -0.5) *
                                                            getScaleMatrix(0.1f)),
                                    56);
    p0_1->addGlobalTransformKeyframe(GeometricTransformation(getRotationMatrix(-M_PI / 2, 1, 0, 0) * getRotationMatrix(-M_PI, 0, 0, 1) *
                                                             getTranslationMatrix(0.0, 5.0, -0.5) *
                                                             getScaleMatrix(0.1f)),
                                     56);
    p0_2->addGlobalTransformKeyframe(GeometricTransformation(getRotationMatrix(-M_PI / 2, 1, 0, 0) * getRotationMatrix(-M_PI, 0, 0, 1) *
                                                             getTranslationMatrix(0.5, 5.0, -0.5) *
                                                             getScaleMatrix(0.1f)),
                                     56);
    p10->addGlobalTransformKeyframe(GeometricTransformation(getRotationMatrix(-M_PI / 2, 1, 0, 0) * getRotationMatrix(-M_PI, 0, 0, 1) *
                                                            getTranslationMatrix(-0.5, 5.0, -0.5) *
                                                            getScaleMatrix(0.1f)),
                                    500);
    p0_1->addGlobalTransformKeyframe(GeometricTransformation(getRotationMatrix(-M_PI / 2, 1, 0, 0) * getRotationMatrix(-M_PI, 0, 0, 1) *
                                                             getTranslationMatrix(0.0, 5.0, -0.5) *
                                                             getScaleMatrix(0.1f)),
                                     500);
    p0_2->addGlobalTransformKeyframe(GeometricTransformation(getRotationMatrix(-M_PI / 2, 1, 0, 0) * getRotationMatrix(-M_PI, 0, 0, 1) *
                                                             getTranslationMatrix(0.5, 5.0, -0.5) *
                                                             getScaleMatrix(0.1f)),
                                     500);

    viewer.addRenderable(p0_1);
    viewer.addRenderable(p0_2);
    viewer.addRenderable(p10);
}

// Fonction pour initialiser la première tortue à chapeau
void initTurtle(Viewer &viewer, ShaderProgramPtr texShader, MaterialPtr tortueMaterial)
{
    // Créer les parties de la tortue
    auto patearrieredroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieredroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto patearrieregauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieregauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantdroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantdroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantgauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantgauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto tete = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/tortueteteaucasou.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto carapace = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/carapace.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto chapeau = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/accessoire/chapeaugrandseul.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/Texture_TopHat.png"); // chapeau avec texture

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(carapace, tete);
    HierarchicalRenderable::addChild(carapace, patearrieredroite);
    HierarchicalRenderable::addChild(carapace, patearrieregauche);
    HierarchicalRenderable::addChild(carapace, pateavantdroite);
    HierarchicalRenderable::addChild(carapace, pateavantgauche);
    HierarchicalRenderable::addChild(tete, chapeau);

    // Transformations
    chapeau->setLocalTransform(getScaleMatrix(0.4, 0.4, 0.4) * getTranslationMatrix(0.0f, 2.0f, 16.0f));
    carapace->setGlobalTransform(getTranslationMatrix(0.0, -0.9, 0.0) * getScaleMatrix(0.1));

    // Animation
    float begin = 7.0f; // La tortue commence à bouger à la seconde 7
    float totalDuration = 16.0f; // Durée totale de l'animation de déplacement

    // Animation de la carapace (déplacement de la tortue)
    // Déplacement de la tortue de -6 à +2 sur l'axe Z entre les secondes 7 et 23 puis reste sur place jusqu'à la seconde 29
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, -6.0f), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), begin);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, 2.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), begin + totalDuration);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, 2.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 29);
    // La tortue disparaît ensuite à partir de la seconde 30 en montant sur l'axe Y
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, 10.0, 10.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 30);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, 10.0, 10.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 500);

    // Animation des pattes
    float legAngle = 5.0f;

    // Patte avant gauche
    // Les pattes bougent en alternance : quand une patte va vers l'avant, la patte diagonale va vers l'arrière
    // gauche et droite vont vers l'avant ensemble, de même pour les pattes arrière
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    // Les pattes bougent aussi de haut en bas pour simuler la marche
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    // Animation de la tête
    float headAngle = -2.0f;
    // La tête bouge de haut en bas pendant que la tortue marche
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(headAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0)), glm::vec3(1)), 4.0f);

    viewer.addRenderable(carapace);
}

// Fonction pour initialiser la deuxième tortue à rose
void initTurtle2(Viewer &viewer, ShaderProgramPtr texShader, MaterialPtr tortueMaterial)
{
    // Créer les parties de la tortue
    auto patearrieredroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieredroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto patearrieregauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieregauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantdroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantdroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantgauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantgauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto tete = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/tortueteteaucasou.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto carapace = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/tortue2v2.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/tortue2v2.jpg"); // carapace rose

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(carapace, tete);
    HierarchicalRenderable::addChild(carapace, patearrieredroite);
    HierarchicalRenderable::addChild(carapace, patearrieregauche);
    HierarchicalRenderable::addChild(carapace, pateavantdroite);
    HierarchicalRenderable::addChild(carapace, pateavantgauche);

    // Transformations
    carapace->setGlobalTransform(getTranslationMatrix(0.0, -0.9, 0.0) * getScaleMatrix(0.1));

    // Animation
    float begin = 28.0f; // La tortue commence à bouger à la seconde 28
    float totalDuration = 12.0f; // Durée totale de l'animation de déplacement

    // Animation de la carapace (déplacement de la tortue)
    // Déplacement de la tortue de -6 à +2 sur l'axe Z entre les secondes 28 et 40 puis reste sur place jusqu'à la seconde 48
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, -6.0f), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), begin);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, 2.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), begin + totalDuration);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, 2.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 48);
    // La tortue disparaît ensuite à partir de la seconde 49 en montant sur l'axe Y
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, 10.0, 10.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 49);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, 10.0, 10.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 500);

    // Animation des pattes
    float legAngle = 5.0f;

    // Patte avant gauche
    // Les pattes bougent en alternance : quand une patte va vers l'avant, la patte diagonale va vers l'arrière
    // gauche et droite vont vers l'avant ensemble, de même pour les pattes arrière
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    // Les pattes bougent aussi de haut en bas pour simuler la marche
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    // Animation de la tête
    float headAngle = -2.0f;
    // La tête bouge de haut en bas pendant que la tortue marche
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(headAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0)), glm::vec3(1)), 4.0f);

    viewer.addRenderable(carapace);
}

// Fonction pour initialiser la troisième tortue à collier de perles
void initTurtle3(Viewer &viewer, ShaderProgramPtr texShader, MaterialPtr tortueMaterial)
{
    // Créer les parties de la tortue
    auto patearrieredroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieredroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto patearrieregauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieregauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantdroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantdroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantgauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantgauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto tete = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/troturelongcou.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto carapace = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/carapace.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto collier = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/collierboulesmemeobjet.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/perle.png"); // collier avec texture de perles

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(carapace, tete);
    HierarchicalRenderable::addChild(carapace, patearrieredroite);
    HierarchicalRenderable::addChild(carapace, patearrieregauche);
    HierarchicalRenderable::addChild(carapace, pateavantdroite);
    HierarchicalRenderable::addChild(carapace, pateavantgauche);
    HierarchicalRenderable::addChild(tete, collier);

    // Transformations
    carapace->setGlobalTransform(getTranslationMatrix(0.0, -0.9, 0.0) * getScaleMatrix(0.1));
    collier->setLocalTransform(getTranslationMatrix(0.0, 0.0, 4.8));

    // Animation
    float begin = 42.0f; // La tortue commence à bouger à la seconde 42
    float totalDuration = 12.0f; // Durée totale de l'animation de déplacement

    // Animation de la carapace (déplacement de la tortue)
    // Déplacement de la tortue de -6 à +2 sur l'axe Z entre les secondes 44 et 56 puis reste sur place jusqu'à la seconde 62
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, -6.0f), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), begin);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, -6.0f), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), begin + 2);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, 2.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), begin + totalDuration + 2);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, -0.9, 2.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 62);
    // La tortue disparaît ensuite à partir de la seconde 63 en montant sur l'axe Y
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, 10.0, -10.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 63);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0, 10.0, -10.0), glm::quat{}, glm::vec3(0.1, 0.1, 0.1)), 100);

    // Animation du collier (il est attaché au cou mais tombe à la seconde 56 et disparait en dehors de l'écran)
    collier->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0.0, 0.0, 4.8), glm::quat{}, glm::vec3(1.0)), 0);
    collier->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0.0, 0.0, 4.8), glm::quat{}, glm::vec3(1.0)), begin + totalDuration + 2);
    collier->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0.0, -20.0, 4.8), glm::quat{}, glm::vec3(1.0)), begin + totalDuration + 2.5);
    collier->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0.0, -20.0, 4.8), glm::quat{}, glm::vec3(1.0)), 500);

    // Animation des pattes
    float legAngle = 5.0f;

    // Patte avant gauche
    // Les pattes bougent en alternance : quand une patte va vers l'avant, la patte diagonale va vers l'arrière
    // gauche et droite vont vers l'avant ensemble, de même pour les pattes arrière
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 0.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 1.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(0, 1, 0)), glm::vec3(1)), 2.0f);

    // Les pattes bougent aussi de haut en bas pour simuler la marche
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    pateavantgauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    patearrieredroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    pateavantdroite->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 1.0f);
    patearrieregauche->addLocalTransformKeyframe(GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(-legAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);

    // Animation de la tête
    float headAngle = -2.0f;
    // La tête bouge de haut en bas pendant que la tortue marche
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0)), glm::vec3(1)), 0.0f);
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(headAngle), glm::vec3(1, 0, 0)), glm::vec3(1)), 2.0f);
    tete->addGlobalTransformKeyframe(
        GeometricTransformation(glm::vec3(0), glm::angleAxis(glm::radians(0.0f), glm::vec3(1, 0, 0)), glm::vec3(1)), 4.0f);

    viewer.addRenderable(carapace);
}

// Crée un système dynamique représentant un collier de perles
// Le rendu du collier est activé après "startTime" secondes
// Le rendu est masqué au départ et dévoilé lorsque le système devient actif
// Les perles sont des Particles reliées par des ressorts formant une structure en anneaux empilés
// Le système dynamique inclut des forces : gravité, amortissement et un trigger d'explosion qui, après un délai, supprime les ressorts et applique une impulsion aux particules.
// Paramètres :
// - viewer : le viewer pour ajouter les shaders
// - startTime : le temps (en secondes) après lequel le collier devient visible et actif
DynamicSystemRenderablePtr createDelayedNecklaceSystem(Viewer &viewer, float startTime)
{
    // Créer le système dynamique avec solveur
    DynamicSystemPtr necklaceSystem = std::make_shared<DynamicSystem>();
    EulerExplicitSolverPtr solver = std::make_shared<EulerExplicitSolver>();
    necklaceSystem->setSolver(solver);
    necklaceSystem->setDt(0.01);

    // Créer un renderable personnalisé avec contrôle du temps
    class DelayedDynamicSystemRenderable : public DynamicSystemRenderable
    {
    private:
        // Temps de début pour activer le système
        float m_startTime;
        // Indicateur d'activation
        bool m_isActive;
        // Renderables des perles et des ressorts
        ParticleListRenderablePtr m_beadsRenderable;
        SpringListRenderablePtr m_springsRenderable;

    public:
        // Constructeur 
        // Prend le système dynamique et le temps de début en paramètres
        // Initialise l'état d'activation à false
        DelayedDynamicSystemRenderable(DynamicSystemPtr system, float startTime)
            : DynamicSystemRenderable(system), m_startTime(startTime), m_isActive(false)
        {
        }

        // Définir les renderables des perles et des ressorts à contrôler
        void setRenderables(ParticleListRenderablePtr beads, SpringListRenderablePtr springs)
        {
            m_beadsRenderable = beads;
            m_springsRenderable = springs;

            // Cacher les renderables au début
            if (m_beadsRenderable)
                // Scale à 0 pour cacher
                m_beadsRenderable->setLocalTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.0f)));
            if (m_springsRenderable)
                // Scale à 0 pour cacher
                m_springsRenderable->setLocalTransform(glm::scale(glm::mat4(1.0f), glm::vec3(0.0f)));
        }

        // Surcharge de la méthode d'animation pour gérer l'activation différée
        void do_animate(float time) override
        {
            // Activer à partir du temps startTime
            if (!m_isActive && time >= m_startTime)
            {
                m_isActive = true;

                // Afficher les renderables
                if (m_beadsRenderable)
                    m_beadsRenderable->setLocalTransform(glm::mat4(1.0f));
                if (m_springsRenderable)
                    m_springsRenderable->setLocalTransform(glm::mat4(1.0f));
            }

            // Ne mettre à jour la physique que quand actif
            if (m_isActive)
            {
                DynamicSystemRenderable::do_animate(time);
            }
        }
    };

    // Créer le renderable avec délai
    auto delayedRenderable = std::make_shared<DelayedDynamicSystemRenderable>(necklaceSystem, startTime);

    // Créer les shaders pour les perles et les ressorts
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(
        "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
        "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");
    ShaderProgramPtr instancedShader = std::make_shared<ShaderProgram>(
        "../../sfmlGraphicsPipeline/shaders/instancedVertex.glsl",
        "../../sfmlGraphicsPipeline/shaders/instancedFragment.glsl");

    viewer.addShaderProgram(flatShader);
    viewer.addShaderProgram(instancedShader);

    // Activer la détection de collision
    necklaceSystem->setCollisionsDetection(true);
    necklaceSystem->setRestitution(0.05f); // faible rebond

    // Sol pour le collier
    glm::vec3 planeNormal(0.0f, 1.0f, 0.0f);
    glm::vec3 planePoint(0.0f, -1.2f, 2.0f);
    PlanePtr ground = std::make_shared<Plane>(planeNormal, planePoint);
    necklaceSystem->addPlaneObstacle(ground);

    // Paramètres du collier
    int numberOfLevels = 12; // Nombre de niveaux de perles empilés
    int beadsPerLevel = 10; // Nombre de perles par niveau
    float verticalSpacing = 0.15f; // Espacement vertical entre les niveaux
    float levelRadius = 0.2f; // Rayon de chaque niveau circulaire
    float startHeight = -0.8f; // Hauteur de départ du collier

    float beadRadius = 0.05f; // Rayon de chaque perle
    float beadMass = 0.4f; // Masse de chaque perle

    float springStiffness = 200.0f; // Raideur des ressorts
    float springDamping = 5.0f; // Amortissement des ressorts

    float angleStep = (2.0f * M_PI) / beadsPerLevel; // Angle entre chaque perle sur un niveau
    float chordLength = 2.0f * levelRadius * sin(angleStep / 2.0f); // Longueur de la corde entre deux perles adjacentes
    float springRestLength = chordLength; // Longueur au repos des ressorts horizontaux

    float verticalSpringStiffness = 300.0f; // Raideur des ressorts verticaux
    float verticalSpringRestLength = verticalSpacing; // Longueur au repos des ressorts verticaux

    // Stocker les perles par niveau
    std::vector<std::vector<ParticlePtr>> beadsByLevel(numberOfLevels);
    // Stocker toutes les perles et ressorts
    std::vector<ParticlePtr> allBeads;
    std::list<SpringForceFieldPtr> allSprings;

    // Créer les perles
    for (int level = 0; level < numberOfLevels; ++level)
    {
        // Calculer la hauteur actuelle du niveau
        float currentHeight = startHeight - (level * verticalSpacing);

        for (int beadIndex = 0; beadIndex < beadsPerLevel; ++beadIndex)
        {
            float angle = (2.0f * M_PI * beadIndex) / beadsPerLevel;

            // Position initiale (sera cachée jusqu'à 57 secondes)
            glm::vec3 position(
                levelRadius * cos(angle) + 0.0f,
                currentHeight + 1.0f,
                levelRadius * sin(angle) + 3.5f);

            // Vitesse initiale nulle
            glm::vec3 velocity(0.0f);

            // Créer la perle et l'ajouter au système
            ParticlePtr bead = std::make_shared<Particle>(position, velocity, beadMass, beadRadius);
            beadsByLevel[level].push_back(bead);
            allBeads.push_back(bead);
            necklaceSystem->addParticle(bead);
        }
    }

    // Ressorts horizontaux
    for (int level = 0; level < numberOfLevels; ++level)
    {
        for (int beadIndex = 0; beadIndex < beadsPerLevel; ++beadIndex)
        {
            // Prochain index avec wrap-around
            int nextIndex = (beadIndex + 1) % beadsPerLevel;

            // Créer le ressort entre la perle actuelle et la suivante du même niveau puis l'ajouter au système
            SpringForceFieldPtr spring = std::make_shared<SpringForceField>(
                beadsByLevel[level][beadIndex],
                beadsByLevel[level][nextIndex],
                springStiffness,
                springRestLength,
                springDamping);
            allSprings.push_back(spring);
            necklaceSystem->addForceField(spring);
        }
    }

    // Ressorts verticaux
    for (int level = 0; level < numberOfLevels - 1; ++level)
    {
        for (int beadIndex = 0; beadIndex < beadsPerLevel; ++beadIndex)
        {
            // Créer le ressort entre la perle actuelle et celle du niveau supérieur puis l'ajouter au système
            SpringForceFieldPtr spring = std::make_shared<SpringForceField>(
                beadsByLevel[level][beadIndex],
                beadsByLevel[level + 1][beadIndex],
                verticalSpringStiffness,
                verticalSpringRestLength,
                springDamping);
            allSprings.push_back(spring);
            necklaceSystem->addForceField(spring);
        }
    }

    // Créer les renderables
    ParticleListRenderablePtr beadsRenderable = std::make_shared<ParticleListRenderable>(
        instancedShader, allBeads);
    HierarchicalRenderable::addChild(delayedRenderable, beadsRenderable);

    SpringListRenderablePtr springsRenderable = std::make_shared<SpringListRenderable>(
        flatShader, allSprings);
    HierarchicalRenderable::addChild(delayedRenderable, springsRenderable);

    // Donner les références des renderables au système retardé
    delayedRenderable->setRenderables(beadsRenderable, springsRenderable);

    // Gravité
    ConstantForceFieldPtr gravityForceField = std::make_shared<ConstantForceField>(
        necklaceSystem->getParticles(),
        glm::vec3(0.0f, -9.81f, 0.0f));
    necklaceSystem->addForceField(gravityForceField);

    // Amortissement
    float dampingCoefficient = 1.5f;
    DampingForceFieldPtr dampingForceField = std::make_shared<DampingForceField>(
        necklaceSystem->getParticles(),
        dampingCoefficient);
    necklaceSystem->addForceField(dampingForceField);

    // Trigger d'explosion : supprime les ressorts et applique une impulsion aux particules après un délai (explosionDelay)
    // Il regarde le temps écoulé via "system->getDt()" jusqu'à "explosionDelay" puis :
    // Il remplace la liste de forceFields par celles non-spring pour désactiver les ressorts et laisser les particules libres
    // Ensuite, il applique une impulsion aléatoire normale à chaque perle pour créer un effet d'explosion
    struct ExplosionTriggerForceField : public ForceField
    {
        // Référence au système dynamique
        DynamicSystemPtr system;
        // Temps écoulé depuis le début
        float elapsed;
        // Délai avant l'explosion
        float explosionDelay;
        // Indicateur si l'explosion a été déclenchée
        bool isTriggered;
        // Générateur de nombres aléatoires
        std::mt19937 rng;
        // Distribution uniforme pour les directions
        std::uniform_real_distribution<float> uni;

        // Constructeur
        // Initialise les membres et le générateur de nombres aléatoires
        // "delay" est le délai avant l'explosion
        // "sys" est le système dynamique auquel la force est appliquée
        // L'initialisation de "elapsed" à 0.0f et "isTriggered" à false
        // La distribution uniforme est entre -1.0f et 1.0f pour générer des directions aléatoires
        ExplosionTriggerForceField(DynamicSystemPtr sys, float delay)
            : system(sys), elapsed(0.0f), explosionDelay(delay), isTriggered(false),
              rng(static_cast<unsigned int>(std::time(nullptr))), uni(-1.0f, 1.0f)
        {
        }

        // Surcharge de la méthode pour appliquer la force
        void do_addForce() override
        {
            // Ne rien faire si déjà déclenché
            if (isTriggered)
                return;

            // Mettre à jour le temps écoulé
            elapsed += system->getDt();
            // Attendre jusqu'au délai d'explosion
            if (elapsed < explosionDelay)
                return;

            // Déclencher l'explosion
            // Supprimer les ressorts
            std::vector<ForceFieldPtr> keep;
            for (const ForceFieldPtr &f : system->getForceFields())
            {
                // Garder seulement les forces non-spring
                SpringForceFieldPtr sf = std::dynamic_pointer_cast<SpringForceField>(f);
                if (!sf)
                    keep.push_back(f);
            }
            // Mettre à jour les forces du système
            system->setForceFields(keep);

            // Impulsion d'explosion
            float explosionStrength = 4.0f; // Force de l'explosion
            for (ParticlePtr p : system->getParticles())
            {
                // Générer une direction aléatoire normalisée
                glm::vec3 dir(uni(rng), uni(rng), uni(rng));
                // Assurer que la direction n'est pas nulle
                if (glm::length2(dir) < 1e-6f)
                    dir = glm::vec3(0, 1, 0);
                dir = glm::normalize(dir);

                // Calculer et appliquer l'impulsion en fonction de la masse
                glm::vec3 impulse = dir * (explosionStrength / p->getMass());
                // Appliquer l'impulsion à chaque particule
                p->incrVelocity(impulse);
            }

            isTriggered = true;
        }
    };

    float explosionDelay = 2.0f; // Explose 2 secondes après l'apparition
    auto trigger = std::make_shared<ExplosionTriggerForceField>(necklaceSystem, explosionDelay); // Créer le trigger
    // Ajouter le trigger au système
    necklaceSystem->addForceField(trigger);

    return delayedRenderable;
}

// Initialise le podium final et positionne les tortues qui sont dessus
void initPodium(Viewer &viewer, ShaderProgramPtr texShader, MaterialPtr tortueMaterial, MaterialPtr podiumMaterial)
{
    // Créer le podium
    auto podium = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/plane/podium.obj",
        podiumMaterial, "./../../sfmlGraphicsPipeline/textures/CATWALK/podium.png");
    // Positionner le podium à la fin de l'animation
    podium->addGlobalTransformKeyframe(
        GeometricTransformation(getRotationMatrix(-M_PI / 2, 0, 1, 0) *
                             getTranslationMatrix(0.0,-0.9, 0.25) *
                             getScaleMatrix(0.33)), 66.0f);
    podium->addGlobalTransformKeyframe(
        GeometricTransformation(getRotationMatrix(-M_PI / 2, 0, 1, 0) *
                             getTranslationMatrix(0.0,-0.9, 0.25) *
                             getScaleMatrix(0.33)), 100.0f);
    viewer.addRenderable(podium);

    // Créer la première tortue (tortue à chapeau)
    auto patearrieredroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieredroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto patearrieregauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieregauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantdroite = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantdroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantgauche = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantgauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto tete = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/tortueteteaucasou.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto carapace = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/carapace.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto chapeau = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/accessoire/chapeaugrandseul.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/Texture_TopHat.png");

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(carapace, tete);
    HierarchicalRenderable::addChild(carapace, patearrieredroite);
    HierarchicalRenderable::addChild(carapace, patearrieregauche);
    HierarchicalRenderable::addChild(carapace, pateavantdroite);
    HierarchicalRenderable::addChild(carapace, pateavantgauche);
    HierarchicalRenderable::addChild(tete, chapeau);

    // Transformations
    chapeau->setLocalTransform(getScaleMatrix(0.4, 0.4, 0.4) * getTranslationMatrix(0.0f, 2.0f, 16.0f));
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(getTranslationMatrix(-1.0, -0.05, 0.0) * getScaleMatrix(0.1)), 66.0f);
    carapace->addGlobalTransformKeyframe(
        GeometricTransformation(getTranslationMatrix(-1.0, -0.05, 0.0) * getScaleMatrix(0.1)), 100.0f);

    viewer.addRenderable(carapace);

    // Créer la deuxième tortue (tortue à rose)
    auto patearrieredroite2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieredroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto patearrieregauche2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieregauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantdroite2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantdroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantgauche2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantgauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto tete2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/tortueteteaucasou.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto carapace2 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/tortue2v2.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/tortue2v2.jpg");

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(carapace2, tete2);
    HierarchicalRenderable::addChild(carapace2, patearrieredroite2);
    HierarchicalRenderable::addChild(carapace2, patearrieregauche2);
    HierarchicalRenderable::addChild(carapace2, pateavantdroite2);
    HierarchicalRenderable::addChild(carapace2, pateavantgauche2);

    // Transformations
    carapace2->setGlobalTransform(getTranslationMatrix(1.0, -0.3, 0.0) * getScaleMatrix(0.1));
    carapace2->addGlobalTransformKeyframe(
        GeometricTransformation(getTranslationMatrix(1.0, -0.3, 0.0) * getScaleMatrix(0.1)), 66.0f);
    carapace2->addGlobalTransformKeyframe(
        GeometricTransformation(getTranslationMatrix(1.0, -0.3, 0.0) * getScaleMatrix(0.1)), 100.0f);

    viewer.addRenderable(carapace2);

    // Créer les parties de la troisième tortue (tortue longue cou)
    auto patearrieredroite3 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieredroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto patearrieregauche3 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/patearrieregauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantdroite3 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantdroite.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto pateavantgauche3 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/pateavantgauche.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto tete3 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/troturelongcou.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    auto carapace3 = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/tortue_decoupee/carapace.obj",
        tortueMaterial, "../../sfmlGraphicsPipeline/textures/13103_pearlturtle_diffuse.jpg");

    // Assembler la hiérarchie
    HierarchicalRenderable::addChild(carapace3, tete3);
    HierarchicalRenderable::addChild(carapace3, patearrieredroite3);
    HierarchicalRenderable::addChild(carapace3, patearrieregauche3);
    HierarchicalRenderable::addChild(carapace3, pateavantdroite3);
    HierarchicalRenderable::addChild(carapace3, pateavantgauche3);

    // Transformations
    carapace3->setGlobalTransform(getTranslationMatrix(0.0, 0.25, 0.0) * getScaleMatrix(0.1));
    carapace3->addGlobalTransformKeyframe(
        GeometricTransformation(getTranslationMatrix(0.0, 0.25, 0.0) * getScaleMatrix(0.1)), 66.0f);
    carapace3->addGlobalTransformKeyframe(
        GeometricTransformation(getTranslationMatrix(0.0, 0.25, 0.0) * getScaleMatrix(0.1)), 100.0f);

    viewer.addRenderable(carapace3);
}

// Configure la scène complète : shaders, caméra, skybox, lumières, matériaux et création des objets (tortues, pingouins, jury, podium, collier...)
// À la fin, la fonction démarre l'animation et définit la boucle temporelle.
void initialize_scene(Viewer &viewer)
{
    // Initialiser les shaders
    ShaderProgramPtr flatShader = std::make_shared<ShaderProgram>(
        "../../sfmlGraphicsPipeline/shaders/flatVertex.glsl",
        "../../sfmlGraphicsPipeline/shaders/flatFragment.glsl");

    ShaderProgramPtr cubeMapShader = std::make_shared<ShaderProgram>(
        "../../sfmlGraphicsPipeline/shaders/cubeMapVertex.glsl",
        "../../sfmlGraphicsPipeline/shaders/cubeMapFragment.glsl");

    ShaderProgramPtr phongShader = std::make_shared<ShaderProgram>(
        "../../sfmlGraphicsPipeline/shaders/phongVertex.glsl",
        "../../sfmlGraphicsPipeline/shaders/phongFragment.glsl");

    ShaderProgramPtr texShader = std::make_shared<ShaderProgram>(
        "../../sfmlGraphicsPipeline/shaders/textureVertex.glsl",
        "../../sfmlGraphicsPipeline/shaders/textureFragment.glsl");

    // Ajouter les shaders au viewer
    viewer.addShaderProgram(texShader);
    viewer.addShaderProgram(flatShader);
    viewer.addShaderProgram(cubeMapShader);
    viewer.addShaderProgram(phongShader);

    // Gestion de la camera
    // Position initiale de la caméra
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0, 0.0, 0.0), glm::vec3(0, 0, 5)), 0);
    // zoom sur les pingouins
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(-1.0, -1.0, 4), glm::vec3(-3, 0, 4)), 2);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(-1.0, -1.0, -4), glm::vec3(-3, 0, -4)), 7);
    // regarde la première tortue qui avance
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(-1, 0.0, -4.0f), glm::vec3(0, -0.9, -6.0f)), 8);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, -1.0f), glm::vec3(0, 0.0, -2.0f)), 12);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(1, -0.8, 2.0f), glm::vec3(0, -0.9, 2.0f)), 23);
    // regarde les juges
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, -0.5, 4.0f), glm::vec3(0, -0.4, 5.0f)), 24);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, -0.5, 4.0f), glm::vec3(0, -0.4, 5.0f)), 26);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 1.0f), glm::vec3(0, -0.4, 5.0f)), 27);
    // se retourne vers la deuxième tortue
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, -1.0f), glm::vec3(0, -0.9, -6.0f)), 29);
    // la suit
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, -1.0f), glm::vec3(0, -0.9, -6.0f)), 30);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 1.0, -3.0f), glm::vec3(0, -0.9, -4.0f)), 32);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, -3.0f), glm::vec3(0, -0.9, 0.0f)), 35);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 1.0f), glm::vec3(0, -0.9, 2.0f)), 40);
    // regarde les juges encore une fois
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 4.0f), glm::vec3(0, -0.4, 5.0f)), 41);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 4.0f), glm::vec3(0, -0.4, 5.0f)), 42);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 4.0f), glm::vec3(0, -0.9, 2.0f)), 43);
    // Il se retourne vers la tortue perdante
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, -0.6, 3.0f), glm::vec3(0, -0.6, 2.0f)), 43.2);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, -0.6, 3.0f), glm::vec3(0, -0.6, 2.0f)), 43.8);
    // Il regarde la troisième tortue
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 3.0f), glm::vec3(0, -0.9, -6.0f)), 44);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 0.0f), glm::vec3(0, -0.9, -6.0f)), 45);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(-1.2, -0.9, -2.5f), glm::vec3(0.0, 0.0, -2.0f)), 50);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, -0.5f), glm::vec3(0, -0.4, 0.5f)), 53);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(1.0, -0.2, 3.0f), glm::vec3(0, 0.0, 2.5f)), 56);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.5, 3.3f), glm::vec3(0, 0.1, 2.5f)), 57);
    // La caméra regarde les perles qui explosent
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(1.0, 0.0, 4.0f), glm::vec3(0, -1.0, 3.5f)), 57.5);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(1.0, 0.0, 4.0f), glm::vec3(0, -1.0, 3.5f)), 60);
    // Zoom arrière sur la scène des pingouins qui tombent
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 1.0, 4.0f), glm::vec3(0.1, -1.0, 5.0f)), 61);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 1.0, 4.0f), glm::vec3(0.1, -1.0, 5.0f)), 63.5);
    // Zoom avant sur les panneaux
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.2, 4.5f), glm::vec3(0.1, -0.5, 5.0f)), 64.5);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.2, 4.5f), glm::vec3(0.1, -0.5, 5.0f)), 66);
    // Regarde le podium
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 4.5f), glm::vec3(0.05, 0.0, 0.0f)), 67);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 4.5f), glm::vec3(0.05, 0.0, 0.0f)), 67.5);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 2.0f), glm::vec3(0.05, 0.0, 0.0f)), 69);
    viewer.getCamera().addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 0.0, 2.0f), glm::vec3(0.05, 0.0, 0.0f)), 100);

    // Skybox
    std::string skybox_dir = "../../sfmlGraphicsPipeline/textures/skybox2";
    auto cubemap = std::make_shared<CubeMapRenderable>(cubeMapShader, skybox_dir);
    viewer.addRenderable(cubemap);

    // Définir les transformations globales et locales utilisées plus bas
    glm::mat4 globalTransformation, localTransformation;

    // Définir une lumière directionnelle
    glm::vec3 d_direction = glm::normalize(glm::vec3(0.0, 0.0, 10.0));
    glm::vec3 d_ambient = glm::vec3(0.1);
    glm::vec3 d_diffuse = glm::vec3(0.5);
    glm::vec3 d_specular = glm::vec3(0.5);
    glm::vec3 lightPosition = glm::vec3(0.0, 1000.0, 0.0);
    DirectionalLightPtr directionalLight = std::make_shared<DirectionalLight>(d_direction, d_ambient, d_diffuse, d_specular);
    directionalLight->setGlobalTransform(getTranslationMatrix(lightPosition) * directionalLight->getGlobalTransform());

    DirectionalLightRenderablePtr directionalLightRenderable = std::make_shared<DirectionalLightRenderable>(texShader, directionalLight);
    localTransformation = getScaleMatrix(0.5);
    directionalLightRenderable->setLocalTransform(localTransformation);
    viewer.addDirectionalLight(directionalLight);
    viewer.addRenderable(directionalLightRenderable);

    // Définir un projecteur de spot lumineux
    glm::vec3 s_position(0.0, 0.0, 2.0), s_spotDirection = glm::normalize(glm::vec3(0.0, -1.0, 1.0));
    // glm::vec3 s_ambient(0.0,0.0,0.0), s_diffuse(0.0,0.0,0.0), s_specular(0.0,0.0,0.0);
    glm::vec3 s_ambient(0.0), s_diffuse(0.8), s_specular(0.8);
    float s_constant = 1.0, s_linear = 0.0, s_quadratic = 0.0;
    float s_innerCutOff = std::cos(glm::radians(10.0f)), s_outerCutOff = std::cos(glm::radians(11.0f));
    SpotLightPtr spotLight = std::make_shared<SpotLight>(s_position, s_spotDirection,
                                                         s_ambient, s_diffuse, s_specular,
                                                         s_constant, s_linear, s_quadratic,
                                                         s_innerCutOff, s_outerCutOff);

    // Faire bouger le projecteur avec le temps
    viewer.addSpotLight(spotLight);

    // Définir les images clés pour le mouvement du projecteur
    // Le projecteur suit la première tortue : de 0 à 7 secondes elle ne bouge pas puis de 7 à 29 secondes elle suit la tortue
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 2.0), glm::vec3(0, -0.9, -6.0f)), 0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 2.0), glm::vec3(0, -0.9, -6.0f)), 7.0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 6.0), glm::vec3(0, -0.9, 2.0f)), 23.0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 6.0), glm::vec3(0, -0.9, 2.0f)), 29.0);
    // Le projecteur se tourne en dehors de la scène en attendant qu'on ait encore besoin de lui
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 6.0, 2.0), glm::vec3(0, 10.0, 10.0f)), 30.0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 6.0, 2.0), glm::vec3(0, 10.0, 10.0f)), 41.0);
    // Le projecteur suit la troisième tortue de 42 à 66 secondes
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 2.0), glm::vec3(0, -0.9, -6.0f)), 42.0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 2.0), glm::vec3(0, -0.9, -6.0f)), 44.0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 6.0), glm::vec3(0, -0.9, 2.5f)), 56.0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 6.0), glm::vec3(0, -0.9, 2.5f)), 65.0);
    // Le projecteur éclaire le podium à la fin
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 1.0), glm::vec3(0, 0.0, 0.0f)), 66.0);
    spotLight->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 1.0), glm::vec3(0, 0.0, 0.0f)), 100);

    auto spotLightRenderable = std::make_shared<SpotLightRenderable>(texShader, spotLight);
    viewer.addRenderable(spotLightRenderable);

    // Ajouter un projecteur sur la deuxième tortue
    SpotLightPtr spotLight2 = std::make_shared<SpotLight>(s_position, s_spotDirection,
                                                          s_ambient, s_diffuse, s_specular,
                                                          s_constant, s_linear, s_quadratic,
                                                          s_innerCutOff, s_outerCutOff);

    // Make the spotlight move with time
    viewer.addSpotLight(spotLight2);

    // Définir les images clés pour le mouvement du projecteur
    // Le projecteur n'est pas sur la scène de 0 à 27 secondes
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 2.0), glm::vec3(0, 10.0, -6.0f)), 0);
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 2.0), glm::vec3(0, 10.0, -6.0f)), 27.0);
    // Le projecteur suit la deuxième tortue de 28 à 45 secondes
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 2.0), glm::vec3(0, -0.9, -6.0f)), 28.0);
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 6.0), glm::vec3(0, -0.9, 2.0f)), 40.0);
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 3.0, 6.0), glm::vec3(0, -0.9, 2.0f)), 45.0);
    // Le projecteur se tourne en dehors de la scène en attendant qu'on ait encore besoin de lui
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 2.0), glm::vec3(0, 10.0, 10.0f)), 46.0);
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 2.0), glm::vec3(0, 10.0, 10.0f)), 65.0);
    // Le projecteur éclaire le podium à la fin
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(-2.0, 3.0, 1.0), glm::vec3(-1.0, 0.0, 0.0f)), 66.0);
    spotLight2->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(-2.0, 3.0, 1.0), glm::vec3(-1.0, 0.0, 0.0f)), 100);

    auto spotLightRenderable2 = std::make_shared<SpotLightRenderable>(texShader, spotLight2);
    viewer.addRenderable(spotLightRenderable2);

    // Ajoute un projecteur sur les juges
    SpotLightPtr spotLight3 = std::make_shared<SpotLight>(s_position, s_spotDirection,
                                                          s_ambient, s_diffuse, s_specular,
                                                          s_constant, s_linear, s_quadratic,
                                                          s_innerCutOff, s_outerCutOff);

    // Faire bouger le projecteur avec le temps
    viewer.addSpotLight(spotLight3);

    // De 0 à 100 secondes, le projecteur éclaire les juges
    spotLight3->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 5.0), glm::vec3(0, -1.0, 5.5f)), 0);
    spotLight3->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 5.0), glm::vec3(0, -1.0, 5.5f)), 100);

    auto spotLightRenderable3 = std::make_shared<SpotLightRenderable>(texShader, spotLight3);
    viewer.addRenderable(spotLightRenderable3);

    // Ajoute une quatrième spotlight pour le podium final
    SpotLightPtr spotLight4 = std::make_shared<SpotLight>(s_position, s_spotDirection,
                                                          s_ambient, s_diffuse, s_specular,
                                                          s_constant, s_linear, s_quadratic,
                                                          s_innerCutOff, s_outerCutOff);

    // Faire bouger le projecteur avec le temps
    viewer.addSpotLight(spotLight4);

    // De 0 à 65 secondes, le projecteur n'est pas sur la scène
    spotLight4->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 2.0), glm::vec3(0, 10.0, -6.0f)), 0);
    spotLight4->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(0.0, 5.0, 2.0), glm::vec3(0, 10.0, -6.0f)), 65.0);
    // Le projecteur éclaire le podium à la fin
    spotLight4->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(2.0, 3.0, 1.0), glm::vec3(1.0, 0.0, 0.0f)), 66.0);
    spotLight4->addGlobalTransformKeyframe(lookAtUpModel(glm::vec3(2.0, 3.0, 1.0), glm::vec3(1.0, 0.0, 0.0f)), 100);

    auto spotLightRenderable4 = std::make_shared<SpotLightRenderable>(texShader, spotLight4);
    viewer.addRenderable(spotLightRenderable4);

    // Répertoires des skybox
    std::string diffuse_skybox_dir = "../../sfmlGraphicsPipeline/textures/skybox/diffuse";
    std::string specular_skybox_dir = "../../sfmlGraphicsPipeline/textures/skybox/glossy";

    // Matériaux
    // Définit les matériaux utilisés pour les tortues, les pingouins, la scène et les bancs
    glm::vec3 mAmbient1(1.0), mDiffuse1(1.0), mSpecular1(1.0);
    float mShininess1 = 128.0 * 0.6;
    MaterialPtr tortueMaterial = std::make_shared<Material>(mAmbient1, mDiffuse1, mSpecular1, mShininess1);

    glm::vec3 mAmbient2(1.0), mDiffuse2(1.0), mSpecular2(1.0);
    float mShininess2 = 128.0 * 0.6;
    MaterialPtr pingouinMaterial = std::make_shared<Material>(mAmbient2, mDiffuse2, mSpecular2, mShininess2);

    glm::vec3 mAmbient3(1.0), mDiffuse3(1.0), mSpecular3(1.0);
    float mShininess3 = 128.0 * 0.088;
    MaterialPtr stageMaterial = std::make_shared<Material>(mAmbient3, mDiffuse3, mSpecular3, mShininess3);

    glm::vec3 mAmbient4(0.7), mDiffuse4(0.7), mSpecular4(0.7);
    float mShininess4 = 128.0 * 0.06;
    MaterialPtr benchMaterial = std::make_shared<Material>(mAmbient4, mDiffuse4, mSpecular4, mShininess4);

    // Créer les objets de la scène
    // Scène et bancs du juges
    auto stage = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/stage/catwalk.obj",
        stageMaterial, "./../../sfmlGraphicsPipeline/textures/CATWALK/catwalk_texture.png");
    stage->setLocalTransform(getTranslationMatrix(0.0, 0.0, 0.0) * getScaleMatrix(1));
    viewer.addRenderable(stage);

    auto bench = std::make_shared<TexturedLightedMeshRenderable>(
        texShader, "../../sfmlGraphicsPipeline/meshes/plane/bench.obj",
        benchMaterial, "./../../sfmlGraphicsPipeline/textures/CATWALK/marble.jpg", 1);
    bench->setLocalTransform(getRotationMatrix(-M_PI / 2, 0, 1, 0) *
                             getTranslationMatrix(5.0, -0.9, 0.0) *
                             getScaleMatrix(0.35));
    viewer.addRenderable(bench);

    // Création et gestion de la tortue à chapeau
    initTurtle(viewer, texShader, tortueMaterial);

    // Création et gestion des pingouins du public
    createAllPenguins(viewer, texShader, pingouinMaterial);

    // Création et gestion du jury
    initJury(viewer, texShader, pingouinMaterial);

    // Création et gestion de la deuxième tortue (tortue à rose)
    initTurtle2(viewer,texShader,tortueMaterial);

    // Création et gestion de la troisième tortue (tortue longue cou)
    initTurtle3(viewer, texShader, tortueMaterial);

    // Création et gestion du podium final
    initPodium(viewer, texShader, tortueMaterial, stageMaterial);

    // Création et gestion du collier de perles avec explosion des perles
    float necklaceStartTime = 56.5f; // Temps auquel le collier commence à apparaître
    DynamicSystemRenderablePtr necklaceSystemRenderable = createDelayedNecklaceSystem(viewer, necklaceStartTime); // Crée le système dynamique avec un délai
    viewer.addRenderable(necklaceSystemRenderable);

    // Démarrer l'animation
    float totalDuration = 75.0f;
    viewer.startAnimation();
    // Boucle l'animation au bout de totalDuration secondes
    viewer.setAnimationLoop(true, totalDuration);
}

int main()
{
    Viewer viewer(1280, 720, glm::vec4(0.8, 0.8, 0.8, 1.0));
    initialize_scene(viewer);

    while (viewer.isRunning())
    {
        viewer.handleEvent();
        viewer.animate();
        viewer.draw();
        viewer.display();
    }

    return EXIT_SUCCESS;
}