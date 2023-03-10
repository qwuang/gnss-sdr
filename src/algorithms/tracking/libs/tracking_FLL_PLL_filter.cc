/*!
 * \file tracking_FLL_PLL_filter.cc
 * \brief Implementation of a hybrid FLL and PLL filter for tracking carrier loop
 * \author Javier Arribas, 2011. jarribas(at)cttc.es
 *
 * Class that implements hybrid FLL and PLL filter for tracking carrier loop
 * Filter design (Kaplan 2nd ed., Pag. 181 Fig. 181)
 *
 * -----------------------------------------------------------------------------
 *
 * GNSS-SDR is a Global Navigation Satellite System software-defined receiver.
 * This file is part of GNSS-SDR.
 *
 * Copyright (C) 2010-2020  (see AUTHORS file for a list of contributors)
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * -----------------------------------------------------------------------------
 */

#include "tracking_FLL_PLL_filter.h"


void Tracking_FLL_PLL_filter::set_params(float fll_bw_hz, float pll_bw_hz, int order)
{
    /*
     * Filter design (Kaplan 2nd ed., Pag. 181 Fig. 181)
     */
    d_order = order;
    if (d_order == 3)
        {
            /*
             *  3rd order PLL with 2nd order FLL assist
             */
            d_pll_b3 = 2.400;
            d_pll_a3 = 1.100;
            d_pll_a2 = 1.414;
            d_pll_w0p = pll_bw_hz / 0.7845F;
            d_pll_w0p2 = d_pll_w0p * d_pll_w0p;
            d_pll_w0p3 = d_pll_w0p2 * d_pll_w0p;

            d_pll_w0f = fll_bw_hz / 0.53F;
            d_pll_w0f2 = d_pll_w0f * d_pll_w0f;
        }
    else
        {
            /*
             * 2nd order PLL with 1st order FLL assist
             */
            d_pll_a2 = 1.414;
            d_pll_w0p = pll_bw_hz / 0.53F;
            d_pll_w0p2 = d_pll_w0p * d_pll_w0p;
            d_pll_w0f = fll_bw_hz / 0.25F;
        }
}


void Tracking_FLL_PLL_filter::initialize(float d_acq_carrier_doppler_hz)
{
    if (d_order == 3)
        {
            d_pll_x = 2.0F * d_acq_carrier_doppler_hz;
            d_pll_w = 0;
        }
    else
        {
            d_pll_w = d_acq_carrier_doppler_hz;
            d_pll_x = 0;
        }
}


float Tracking_FLL_PLL_filter::get_carrier_error(float FLL_discriminator, float PLL_discriminator, float correlation_time_s)
{
    float carrier_error_hz;
    if (d_order == 3)
        {
            /*
             * 3rd order PLL with 2nd order FLL assist
             */
            d_pll_w = d_pll_w + correlation_time_s * (d_pll_w0p3 * PLL_discriminator + d_pll_w0f2 * FLL_discriminator);
            d_pll_x = d_pll_x + correlation_time_s * (0.5F * d_pll_w + d_pll_a2 * d_pll_w0f * FLL_discriminator + d_pll_a3 * d_pll_w0p2 * PLL_discriminator);
            carrier_error_hz = 0.5F * d_pll_x + d_pll_b3 * d_pll_w0p * PLL_discriminator;
        }
    else
        {
            /*
             * 2nd order PLL with 1st order FLL assist
             */
            const float pll_w_new = d_pll_w + PLL_discriminator * d_pll_w0p2 * correlation_time_s + FLL_discriminator * d_pll_w0f * correlation_time_s;
            carrier_error_hz = 0.5F * (pll_w_new + d_pll_w) + d_pll_a2 * d_pll_w0p * PLL_discriminator;
            d_pll_w = pll_w_new;
            /* std::cout << " d_pll_w = " << carrier_error_hz << ", pll_w_new = " << pll_w_new
                      << ", PLL_discriminator=" << PLL_discriminator
                      << " FLL_discriminator =" << FLL_discriminator
                      << " correlation_time_s = " << correlation_time_s << "\r\n"; */
        }

    return carrier_error_hz;
}
