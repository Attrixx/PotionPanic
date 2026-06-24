# QTE — Sprites (identité « Apothicaire arcane »)

Sources des sprites de l'UI QTE. Voir la direction visuelle dans
[`Docs/QTE_Motion.md`](../../Docs/QTE_Motion.md).

- `svg/` — sources éditables (vectoriel).
- `png/` — rendus 512×512, fond transparent, **prêts à importer comme `Texture2D`** dans Unreal.

Pour re-générer les PNG après édition d'un SVG : `@resvg/resvg-js` (Node), rendu `width=512`,
fond `rgba(0,0,0,0)`.

## Sprites

| Fichier | Rôle | Type QTE | Note d'usage |
|---|---|---|---|
| `Frame_Medallion` | cadre laiton + verre arcane | tous | plaque de fond réutilisable, le contenu se pose dessus |
| `Ring_Progress_Gold` | anneau de progression | tous | masquer par angle via matériau (param scalaire `Progress`) |
| `Hold_Vial_Glass` | fiole vide (verre + bouchon) | Hold | se pose **devant** le liquide |
| `Hold_Vial_FillMask` | masque blanc du liquide | Hold | drive un matériau de remplissage vertical (`FillAmount`) |
| `Mash_Mortar` | mortier + contenu | Mash | fixe |
| `Mash_Pestle` | pilon | Mash | sprite séparé → anim de punch (`Anim_Punch`) |
| `Press_Seal` | sceau runique | Press | poser un `TextBlock` (touche) au centre |
| `Reagent_Rune` | réactif/rune | Sequence/Chain | tinter lit/dim par matériau (`Lit`) |
| `Grade_Star` | révélation Parfait | fin | `Anim_PerfectBurst` |
| `Grade_Check` | révélation Bien | fin | `Anim_Good` |
| `Grade_Crack` | révélation Raté | fin | `Anim_Fail` |

## Compositing d'une carte (UMG)

De l'arrière vers l'avant :
1. `Frame_Medallion` (fond)
2. `Ring_Progress_Gold` (matériau masqué par `State.StepProgress`)
3. contenu selon le type : fiole (`Hold_Vial_FillMask` derrière + `Hold_Vial_Glass` devant),
   ou `Mash_Mortar` + `Mash_Pestle`, ou `Press_Seal`, ou rangée de `Reagent_Rune`
4. overlay de grade en fin de QTE

## Import Unreal

Glisser les `png/` dans `Content/UI/QTE/Textures/`. Réglages conseillés :
`Texture Group = UI`, `sRGB = on` (sauf `Hold_Vial_FillMask` → `sRGB = off`, c'est un masque),
`Compression = UserInterface2D (RGBA)`, `Mip Gen = NoMipmaps`.
