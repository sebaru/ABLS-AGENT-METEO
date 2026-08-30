/******************************************************************************************************************************/
/* ABLS-AGENT-METEO/src/meteo.c            Gestion des previsions meteo via l'API meteo-concept                               */
/* Projet Abls-Habitat                   Gestion d'habitat                                                27.08.2026 09:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * meteo.c
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

 #include <stdio.h>
 #include <string.h>
 #include <time.h>

 #include "meteo.h"

/******************************************************************************************************************************/
/* Meteo_create_mnemos: Cree les mnemoniques de l'agent                                                                       */
/* Entrée: l'agent                                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Meteo_create_mnemos ( struct ABLS_AGENT *agent )
  { struct ABLS_METEO_VARS *vars = agent->vars;

    vars->sunrise = Mnemo_create_HORLOGE ( agent, "SUNRISE", "Horloge du levé du soleil" );
    vars->sunset  = Mnemo_create_HORLOGE ( agent, "SUNSET",  "Horloge du couché du soleil" );

    for (gint cpt=0; cpt<METEO_NBR_DAYS; cpt++)
     { gchar acronyme[64];
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_WEATHER", cpt );
       vars->Weather[cpt] = Mnemo_create_AI ( agent, acronyme, "Météo prévisionnelle", "code", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MIN", cpt );
       vars->Temp_min[cpt] = Mnemo_create_AI ( agent, acronyme, "Température minimum", "°C", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_TEMP_MAX", cpt );
       vars->Temp_max[cpt] = Mnemo_create_AI ( agent, acronyme, "Température maximum", "°C", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_PLUIE", cpt );
       vars->Proba_pluie[cpt] = Mnemo_create_AI ( agent, acronyme, "Probabilité de pluie (0-100%)", "%", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_GEL", cpt );
       vars->Proba_gel[cpt] = Mnemo_create_AI ( agent, acronyme, "Probabilité de gel (0-100%)", "%", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_BROUILLARD", cpt );
       vars->Proba_brouillard[cpt] = Mnemo_create_AI ( agent, acronyme, "Probabilité de brouillard (0-100%)", "%", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_70", cpt );
       vars->Proba_vent_70[cpt] = Mnemo_create_AI ( agent, acronyme, "Probabilité de vent > 70km/h  (0-100%)", "%", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_PROBA_VENT_100", cpt );
       vars->Proba_vent_100[cpt] = Mnemo_create_AI ( agent, acronyme, "Probabilité de vent > 100km/h (0-100%)", "%", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT_SI_ORAGE", cpt );
       vars->Proba_vent_orage[cpt] = Mnemo_create_AI ( agent, acronyme, "Vitesse des rafales de vent si orage", "km/h", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_VENT_A_10M", cpt );
       vars->Vent_10m[cpt] = Mnemo_create_AI ( agent, acronyme, "Vent moyen à 10 mètres", "km/h", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_DIRECTION_VENT", cpt );
       vars->Direction_vent[cpt] = Mnemo_create_AI ( agent, acronyme, "Direction du vent", "°", AGENT_ARCHIVE_1_HEURE );
       g_snprintf( acronyme, sizeof(acronyme), "DAY%d_RAFALE_VENT", cpt );
       vars->Rafale_vent[cpt] = Mnemo_create_AI ( agent, acronyme, "Vitesse des rafales de vent", "km/h", AGENT_ARCHIVE_1_HEURE );
     }
  }
/******************************************************************************************************************************/
/* Meteo_get_ephemeride: Récupère l'ephemeride auprès de meteo-concept                                                        */
/* Entrée: l'agent                                                                                                            */
/* Sortie: TRUE si la requete a abouti                                                                                        */
/******************************************************************************************************************************/
 static gboolean Meteo_get_ephemeride ( struct ABLS_AGENT *agent )
  { struct ABLS_METEO_VARS *vars = agent->vars;
    gchar *token      = Agent_config_get_string ( agent, "token" );
    gchar *code_insee = Agent_config_get_string ( agent, "code_insee" );

    gchar *query = g_strdup_printf ( "https://api.meteo-concept.com/api/ephemeride/0?token=%s&insee=%s", token, code_insee );
    if (!query) { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ALERT, "Memory Error" ); return(FALSE); }

    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_DEBUG, "Start getting ephemeride for code_insee '%s'", code_insee );

    JsonNode *response = Http_Get_external ( agent, query );
    g_free(query);
    if (!response)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Ephemeride: no response" );
       return(FALSE);
     }

    gboolean retour   = FALSE;
    gint http_code    = Json_get_int ( response, "http_code" );
    if (http_code != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Ephemeride: status %d", http_code ); }
    else
     { JsonNode *city       = Json_get_object_as_node ( response, "city" );
       JsonNode *ephemeride = Json_get_object_as_node ( response, "ephemeride" );
       if (!ephemeride)
        { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Ephemeride: no 'ephemeride' in response" ); }
       else
        { gchar *city_name = (city ? Json_get_string ( city, "name" ) : NULL);
          gchar *sunrise   = Json_get_string ( ephemeride, "sunrise" );
          gchar *sunset    = Json_get_string ( ephemeride, "sunset" );
          gint heure, minute;

          if ( sunrise && sscanf ( sunrise, "%d:%d", &heure, &minute ) == 2)
           { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO, "%s -> sunrise at %02d:%02d",
                   (city_name ? city_name : "?"), heure, minute );
             Mnemo_delete_HORLOGE_tick ( agent, vars->sunrise );
             Mnemo_create_HORLOGE_tick ( agent, vars->sunrise, heure, minute );
           }
          if ( sunset && sscanf ( sunset, "%d:%d", &heure, &minute ) == 2)
           { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_INFO, "%s ->  sunset at %02d:%02d",
                   (city_name ? city_name : "?"), heure, minute );
             Mnemo_delete_HORLOGE_tick ( agent, vars->sunset );
             Mnemo_create_HORLOGE_tick ( agent, vars->sunset, heure, minute );
           }
          retour = TRUE;
        }
     }
    Json_unref ( response );
    return(retour);
  }
/******************************************************************************************************************************/
/* Meteo_update_forecast: Met a jour une journée de prévision                                                                 */
/* Entrée: le tableau json, l'index, l'element et l'agent                                                                     */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Meteo_update_forecast ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { struct ABLS_AGENT *agent = user_data;
    struct ABLS_METEO_VARS *vars = agent->vars;

    gint day = Json_get_int ( element, "day" );
    if (day < 0 || day >= METEO_NBR_DAYS)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_WARNING, "Forecast: day %d out of range, discarding", day );
       return;
     }

    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_DEBUG, "day %02d -> temp_min=%02d, temp_max=%02d",
          day, Json_get_int ( element, "tmin" ), Json_get_int ( element, "tmax" ) );

    Mqtt_Send_AI ( agent, vars->Weather[day],          1.0*Json_get_int ( element, "weather" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Temp_min[day],         1.0*Json_get_int ( element, "tmin" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Temp_max[day],         1.0*Json_get_int ( element, "tmax" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Proba_pluie[day],      1.0*Json_get_int ( element, "probarain" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Proba_gel[day],        1.0*Json_get_int ( element, "probafrost" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Proba_brouillard[day], 1.0*Json_get_int ( element, "probafog" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Proba_vent_70[day],    1.0*Json_get_int ( element, "probawind70" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Proba_vent_100[day],   1.0*Json_get_int ( element, "probawind100" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Proba_vent_orage[day], 1.0*Json_get_int ( element, "gustx" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Vent_10m[day],         1.0*Json_get_int ( element, "wind10m" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Direction_vent[day],   1.0*Json_get_int ( element, "dirwind10m" ), TRUE );
    Mqtt_Send_AI ( agent, vars->Rafale_vent[day],      1.0*Json_get_int ( element, "gust10m" ), TRUE );
  }
/******************************************************************************************************************************/
/* Meteo_get_forecast: Récupère les prévisions auprès de meteo-concept                                                        */
/* Entrée: l'agent                                                                                                            */
/* Sortie: TRUE si la requete a abouti                                                                                        */
/******************************************************************************************************************************/
 static gboolean Meteo_get_forecast ( struct ABLS_AGENT *agent )
  { gchar *token      = Agent_config_get_string ( agent, "token" );
    gchar *code_insee = Agent_config_get_string ( agent, "code_insee" );

    gchar *query = g_strdup_printf ( "https://api.meteo-concept.com/api/forecast/daily?token=%s&insee=%s", token, code_insee );
    if (!query) { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ALERT, "Memory Error" ); return(FALSE); }

    Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_DEBUG, "Start getting forecast for code_insee '%s'", code_insee );

    JsonNode *response = Http_Get_external ( agent, query );
    g_free(query);
    if (!response)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Forecast: no response" );
       return(FALSE);
     }

    gboolean retour  = FALSE;
    gint http_code   = Json_get_int ( response, "http_code" );
    if (http_code != 200)
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "Forecast: status %d", http_code ); }
    else
     { Json_foreach_array_element ( response, "forecast", Meteo_update_forecast, agent );
       retour = TRUE;
     }
    Json_unref ( response );
    return(retour);
  }
/******************************************************************************************************************************/
/* main: Prend en charge l'agent meteo                                                                                        */
/* Entrée: les paramètres de la ligne de commande                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gint main ( gint argc, gchar *argv[] )
  { Config_add_parameter ( "token",      "TOKEN", "Token de l'API Météo-Concept", CONFIG_STRING );
    Config_add_parameter ( "code-insee", "INSEE", "Code INSEE de la commune",     CONFIG_STRING );
    struct ABLS_AGENT *agent = Agent_init ( argv[0], "meteo", ABLS_AGENT_METEO_VERSION, sizeof(struct ABLS_METEO_VARS), argc, argv );
    struct ABLS_METEO_VARS *vars = agent->vars;

    if (!Agent_config_get_string ( agent, "token" ))
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "ERROR: No token, stopping agent" );
       Agent_end(agent);
     }

    if (!Agent_config_get_string ( agent, "code_insee" ))
     { Info( __func__, agent->agent_classe, agent->agent_tech_id, LOG_ERR, "ERROR: No code_insee, stopping agent" );
       Agent_end(agent);
     }

    Meteo_create_mnemos ( agent );

    Agent_is_ready ( agent );

    while(agent->Agent_run == AGENT_IS_RUNNING)                                              /* On tourne tant que necessaire */
     { Agent_loop ( agent );                                             /* Loop sur l'agent pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       JsonNode *mqtt_local_message;
       while ( (mqtt_local_message = Agent_get_mqtt_local_message ( agent ) ) != NULL )
        { Json_unref ( mqtt_local_message ); }
/****************************************************** Ecoute de l'api *******************************************************/
       JsonNode *mqtt_api_message;
       while ( (mqtt_api_message = Agent_get_mqtt_api_message ( agent ) ) != NULL )
        { Json_unref ( mqtt_api_message ); }
/****************************************************** Interrogation du site *************************************************/
       time_t now = time(NULL);
       if (now >= vars->next_request)
        { gboolean ephemeride_ok = Meteo_get_ephemeride ( agent );
          gboolean forecast_ok   = Meteo_get_forecast ( agent );
          gboolean comm_ok       = (ephemeride_ok && forecast_ok);

          Agent_send_comm_to_master ( agent, comm_ok );
          Agent_set_status ( agent, "%s", (comm_ok ? "Prévisions à jour" : "Site meteo-concept injoignable") );
          vars->next_request = now + (comm_ok ? METEO_POLLING_SEC : METEO_RETRY_SEC);        /* Polling adaptatif si erreur */
        }
     }

    Agent_end(agent);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
