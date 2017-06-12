// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////
//

#ifndef API_MANAGER_AUTH_LIB_GRPC_JWT_VERIFIER_H_
#define API_MANAGER_AUTH_LIB_GRPC_JWT_VERIFIER_H_

extern "C" {
#include "src/core/lib/iomgr/pollset.h"
#include "src/core/lib/json/json.h"

#include <grpc/slice.h>
#include <grpc/support/time.h>
}

namespace google {
namespace api_manager {
namespace auth {


/* --- Constants. --- */

#define GRPC_OPENID_CONFIG_URL_SUFFIX "/.well-known/openid-configuration"
#define GRPC_GOOGLE_SERVICE_ACCOUNTS_EMAIL_DOMAIN "gserviceaccount.com"
#define GRPC_GOOGLE_SERVICE_ACCOUNTS_KEY_URL_PREFIX \
  "www.googleapis.com/robot/v1/metadata/x509"

/* --- jwt_verifier_status. --- */

typedef enum {
  JWT_VERIFIER_OK = 0,
  JWT_VERIFIER_BAD_SIGNATURE,
  JWT_VERIFIER_BAD_FORMAT,
  JWT_VERIFIER_BAD_AUDIENCE,
  JWT_VERIFIER_KEY_RETRIEVAL_ERROR,
  JWT_VERIFIER_TIME_CONSTRAINT_FAILURE,
  JWT_VERIFIER_BAD_SUBJECT,
  JWT_VERIFIER_GENERIC_ERROR
} jwt_verifier_status;

const char *jwt_verifier_status_to_string(jwt_verifier_status status);

/* --- jwt_claims. --- */

typedef struct jwt_claims jwt_claims;

void jwt_claims_destroy(grpc_exec_ctx *exec_ctx, jwt_claims *claims);

/* Returns the whole JSON tree of the claims. */
const grpc_json *jwt_claims_json(const jwt_claims *claims);

/* Access to registered claims in https://tools.ietf.org/html/rfc7519#page-9 */
const char *jwt_claims_subject(const jwt_claims *claims);
const char *jwt_claims_issuer(const jwt_claims *claims);
const char *jwt_claims_id(const jwt_claims *claims);
const char *jwt_claims_audience(const jwt_claims *claims);
gpr_timespec jwt_claims_issued_at(const jwt_claims *claims);
gpr_timespec jwt_claims_expires_at(const jwt_claims *claims);
gpr_timespec jwt_claims_not_before(const jwt_claims *claims);

/* --- jwt_verifier. --- */

typedef struct jwt_verifier jwt_verifier;

typedef struct {
  /* The email domain is the part after the @ sign. */
  const char *email_domain;

  /* The key url prefix will be used to get the public key from the issuer:
     https://<key_url_prefix>/<issuer_email>
     Therefore the key_url_prefix must NOT contain https://. */
  const char *key_url_prefix;
} jwt_verifier_email_domain_key_url_mapping;

/* Globals to control the verifier. Not thread-safe. */
extern gpr_timespec jwt_verifier_clock_skew;
extern gpr_timespec jwt_verifier_max_delay;

/* The verifier can be created with some custom mappings to help with key
   discovery in the case where the issuer is an email address.
   mappings can be NULL in which case num_mappings MUST be 0.
   A verifier object has one built-in mapping (unless overridden):
   GRPC_GOOGLE_SERVICE_ACCOUNTS_EMAIL_DOMAIN ->
   GRPC_GOOGLE_SERVICE_ACCOUNTS_KEY_URL_PREFIX.*/
jwt_verifier *jwt_verifier_create(
    const jwt_verifier_email_domain_key_url_mapping *mappings,
    size_t num_mappings);

/*The verifier must not be destroyed if there are still outstanding callbacks.*/
void jwt_verifier_destroy(jwt_verifier *verifier);

/* User provided callback that will be called when the verification of the JWT
   is done (maybe in another thread).
   It is the responsibility of the callee to call jwt_claims_destroy on
   the claims. */
typedef void (*jwt_verification_done_cb)(grpc_exec_ctx *exec_ctx,
                                              void *user_data,
                                              jwt_verifier_status status,
                                              jwt_claims *claims);

/* Verifies for the JWT for the given expected audience. */
void jwt_verifier_verify(grpc_exec_ctx *exec_ctx,
                              jwt_verifier *verifier,
                              grpc_pollset *pollset, const char *jwt,
                              const char *audience,
                              jwt_verification_done_cb cb,
                              void *user_data);

/* --- TESTING ONLY exposed functions. --- */

jwt_claims *jwt_claims_from_json(grpc_exec_ctx *exec_ctx,
                                           grpc_json *json, grpc_slice buffer);
jwt_verifier_status jwt_claims_check(const jwt_claims *claims,
                                               const char *audience);
const char *jwt_issuer_email_domain(const char *issuer);

} // namespace auth
} // namespace api_manager
} // namespace google

#endif /* GRPC_CORE_LIB_SECURITY_CREDENTIALS_JWT_JWT_VERIFIER_H */