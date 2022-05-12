/*******************************************************************************
 * Copyright 2018 MINRES Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef _SCC_SCV_TR_DB_H_
#define _SCC_SCV_TR_DB_H_
#ifndef HAS_SCV
namespace scv_tr {
#endif

/**
 * @fn void scv_tr_sqlite_init()
 * @brief initializes the infrastructure to use a SQLite based transaction recording database
 *
 */
void scv_tr_sqlite_init();
/**
 * @fn void scv_tr_compressed_init()
 * @brief initializes the infrastructure to use a gzip compressed text based transaction recording database
 *
 * TODO: add a multithreaded writer
 *
 */
void scv_tr_compressed_init();
/**
 * @fn void scv_tr_plain_init()
 * @brief initializes the infrastructure to use a plain text based transaction recording database
 *
 * TODO: add a multithreaded writer
 *
 */
void scv_tr_plain_init();
#ifdef WITH_LZ4
/**
 * @fn void scv_tr_lz4_init()
 * @brief initializes the infrastructure to use a LZ4 compressed text based transaction recording database
 *
 * TODO: add a multithreaded writer
 *
 */
void scv_tr_lz4_init();
#endif
/**
 * @fn void scv_tr_mtc_init()
 * @brief initializes the infrastructure to use a compressed text based transaction recording database with a
 * multithreaded writer
 *
 */
void scv_tr_mtc_init();

#ifdef USE_EXTENDED_DB
/**
 * initializes the infrastructure to use a binary transaction recording database
 */
void scv_tr_binary_init();
/**
 * initializes the infrastructure to use a LevelDB based transaction recording database
 */
void scv_tr_ldb_init();

#endif
#ifndef HAS_SCV
}
#endif
#endif /* _SCC_SCV_TR_DB_H_ */
