/******************************************************************************************************************************/
/* ABLS-AGENT-METEO/include/meteo.h        Header de l'agent Meteo                                                            */
/* Projet Abls-Habitat                   Gestion d'habitat                                                27.08.2026 09:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * meteo.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
 *
 * ABLS-AGENT-METEO is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-AGENT-METEO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-AGENT-METEO; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef _ABLS_METEO_H_
#define _ABLS_METEO_H_

#include <abls-agent-libs/abls-agent-libs.h>

#define METEO_NBR_DAYS       14                                       /* Nombre de jours de prévision de l'API meteo-concept */
#define METEO_POLLING_SEC    3600                                                          /* Interrogation nominale du site */
#define METEO_RETRY_SEC      60                                             /* Interrogation accélérée si communication NOK */

struct ABLS_METEO_VARS {
  time_t next_request;

  JsonNode *sunrise;
  JsonNode *sunset;
  JsonNode *Weather[METEO_NBR_DAYS];
  JsonNode *Temp_min[METEO_NBR_DAYS];
  JsonNode *Temp_max[METEO_NBR_DAYS];
  JsonNode *Proba_pluie[METEO_NBR_DAYS];
  JsonNode *Proba_pluie_mm[METEO_NBR_DAYS];
  JsonNode *Proba_pluie_mm_max[METEO_NBR_DAYS];
  JsonNode *Proba_gel[METEO_NBR_DAYS];
  JsonNode *Proba_brouillard[METEO_NBR_DAYS];
  JsonNode *Proba_vent_70[METEO_NBR_DAYS];
  JsonNode *Proba_vent_100[METEO_NBR_DAYS];
  JsonNode *Proba_vent_orage[METEO_NBR_DAYS];
  JsonNode *Vent_10m[METEO_NBR_DAYS];
  JsonNode *Direction_vent[METEO_NBR_DAYS];
  JsonNode *Rafale_vent[METEO_NBR_DAYS];
};

extern struct ABLS_AGENT *Agent;
extern struct ABLS_METEO_VARS *Agent_vars;

#endif /* _ABLS_METEO_H_ */
/*----------------------------------------------------------------------------------------------------------------------------*/
