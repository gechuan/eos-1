/* vim: set sw=4 sts=4 et foldmethod=syntax : */

/*
 * Copyright (c) 2010, 2011 Danny van Dyk
 * Copyright (c) 2010 Christian Wacker
 *
 * This file is part of the EOS project. EOS is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * EOS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <src/utils/parameters.hh>
#include <src/utils/private_implementation_pattern-impl.hh>
#include <src/utils/wrapped_forward_iterator-impl.hh>

#include <cmath>
#include <map>
#include <vector>

namespace eos
{
    struct Parameter::Template
    {
        std::string name;

        double min, central, max;
    };

    struct Parameter::Data :
        Parameter::Template
    {
        double value;

        Data(const Parameter::Template & t) :
            Parameter::Template(t),
            value(t.central)
        {
        }
    };

    struct Parameters::Data
    {
        std::vector<Parameter::Data> data;
    };

    template class WrappedForwardIterator<Parameters::IteratorTag, Parameter>;

    template <>
    struct Implementation<Parameters>
    {
        std::shared_ptr<Parameters::Data> parameters_data;

        std::map<std::string, unsigned> parameters_map;

        std::vector<Parameter> parameters;

        Implementation(const std::initializer_list<Parameter::Template> & list) :
            parameters_data(new Parameters::Data)
        {
            unsigned idx(0);
            for (auto i(list.begin()), i_end(list.end()) ; i != i_end ; ++i, ++idx)
            {
                parameters_data->data.push_back(Parameter::Data(*i));
                parameters_map[i->name] = idx;
                parameters.push_back(Parameter(parameters_data, idx));
            }
        }

        Implementation(const Implementation & other) :
            parameters_data(new Parameters::Data(*other.parameters_data)),
            parameters_map(other.parameters_map)
        {
            parameters.reserve(other.parameters.size());
            for (unsigned i = 0 ; i != parameters.size() ; ++i)
            {
                parameters.push_back(Parameter(parameters_data, i));
            }
        }
    };

    Parameters::Parameters(Implementation<Parameters> * imp) :
        PrivateImplementationPattern<Parameters>(imp)
    {
    }

    Parameters::~Parameters()
    {
    }

    Parameters
    Parameters::clone() const
    {
        return Parameters(new Implementation<Parameters>(*_imp));
    }

    Parameter
    Parameters::operator[] (const std::string & name) const
    {
        auto i(_imp->parameters_map.find(name));

        if (_imp->parameters_map.end() == i)
            throw UnknownParameterError(name);

        return Parameter(_imp->parameters_data, i->second);
    }

    void
    Parameters::set(const std::string & name, const double & value)
    {
        auto i(_imp->parameters_map.find(name));

        if (_imp->parameters_map.end() == i)
            throw UnknownParameterError(name);

        _imp->parameters_data->data[i->second].value = value;
    }

    Parameters::Iterator
    Parameters::begin() const
    {
        return Parameters::Iterator(_imp->parameters.begin());
    }

    Parameters::Iterator
    Parameters::end() const
    {
        return Parameters::Iterator(_imp->parameters.end());
    }

    bool
    Parameters::operator!= (const Parameters & rhs) const
    {
        return rhs._imp.get() != this->_imp.get();
    }

    Parameters
    Parameters::Defaults()
    {
        return Parameters(new Implementation<Parameters>({
            // Wilson coefficients C1 - C6 at mu = 4.2 GeV to NLL accuary, based on [CMM1997]
            Parameter::Template{"c1",                                            -0.32300000, -0.32300000, -0.32300000},
            Parameter::Template{"c2",                                            +1.00931000, +1.00931000, +1.00931000},
            Parameter::Template{"c3",                                            -0.00522869, -0.00522869, -0.00522869},
            Parameter::Template{"c4",                                            -0.08794730, -0.08794730, -0.08794730},
            Parameter::Template{"c5",                                            +0.00037476, +0.00037476, +0.00037476},
            Parameter::Template{"c6",                                            +0.00105859, +0.00105859, +0.00105859},
            // Wilson coefficients C7 - c10 at mu = 4.2 GeV to NNLL,           based on ?
            Parameter::Template{"Abs{c7}",                                       +0.331,      +0.331,      +0.331     }, // c7eff = -0.306
            Parameter::Template{"Arg{c7}",                                       +M_PI,       +M_PI,       +M_PI      },
            Parameter::Template{"c8",                                            -0.181,      -0.181,      -0.181     }, // c8eff = -0.168
            Parameter::Template{"Abs{c9}",                                       +4.27,       +4.27,       +4.27      },
            Parameter::Template{"Arg{c9}",                                       +0.00,       +0.00,       +0.00      },
            Parameter::Template{"Abs{c10}",                                      +4.17,       +4.17,       +4.17      },
            Parameter::Template{"Arg{c10}",                                      +M_PI,       +M_PI,       +M_PI      },
            // Primed Wilson coefficients are negligible in the SM
            Parameter::Template{"c7prime",                                       +0.0,        +0.0,        +0.0       },
            Parameter::Template{"c9prime",                                       +0.0,        +0.0,        +0.0       },
            Parameter::Template{"c10prime",                                      +0.0,        +0.0,        +0.0       },
            // Factorization scale
            Parameter::Template{"mu",                                            +2.4,        +4.2,        +9.6       },
            // CKM matrix elements, cf. [PDG2008], Eqs. (11.4), (11.5), p. 169 and Eq. (11.26), p. 174
            Parameter::Template{"CKM::A",                                        +0.793,      +0.814,      +0.835     },
            Parameter::Template{"CKM::lambda",                                   +0.2247,     +0.2257,     +0.2266    },
            Parameter::Template{"CKM::rhobar",                                   +0.119,      +0.135,      +0.166     },
            Parameter::Template{"CKM::etabar",                                   +0.332,      +0.349,      +0.364     },
            Parameter::Template{"CKM::|V_cb|",                                   +0.0404,     +0.0417,     +0.0430    },
            // QCD inputs
            Parameter::Template{"QCD::alpha_s(MZ)",                              +0.117620,   +0.117620,   +0.117620  },
            Parameter::Template{"QCD::mu_t",                                     +170.0,      +170.0,      +170.0     },
            Parameter::Template{"QCD::mu_b",                                     +4.2,        +4.2,        +4.2       },
            Parameter::Template{"QCD::mu_c",                                     +1.0,        +1.0,        +1.0       },
            Parameter::Template{"QCD::Lambda",                                   +0.5,        +0.5,        +0.5       },
            // Masses in GeV
            Parameter::Template{"mass::b(MSbar)",                                +4.13,       +4.20,       +4.37      }, // cf. [PDG2008], p. 21
            Parameter::Template{"mass::c",                                       +1.16,       +1.27,       +1.34      }, // cf. [PDG2008], p. 21
            Parameter::Template{"mass::s",                                       +0.00,       +0.00,       +0.00      }, // we neglect m_s throughout, cf. [BHvD2010], Table 1
            Parameter::Template{"mass::t",                                       +169.1,      +171.2,      +173.3     }, // cf. [PDG2008], p. 21
            Parameter::Template{"mass::e",                                       +5.10999e-4, +5.10999e-4, +5.10999e-4}, // cf. [PDG2008], p. 13
            Parameter::Template{"mass::mu",                                      +1.05658e-1, +1.05658e-1, +1.05658e-1}, // cf. [PDG2008], p. 13
            Parameter::Template{"mass::tau",                                     +1.77667,    +1.77684,    +1.77701   }, // cf. [PDG2008], p. 14
            Parameter::Template{"mass::B0",                                      +5.27920,    +5.27953,    +5.27986   }, // cf. [PDG2008], p. 79
            Parameter::Template{"mass::K0",                                      +0.49759,    +0.49761,    +0.49764   }, // cf. [PDG2008], p. 41
            Parameter::Template{"mass::K^*0",                                    +0.89575,    +0.896,      +0.89625   }, // cf. [PDG2008], p. 44
            Parameter::Template{"mass::W",                                       +80.373,     +80.398,     +80.423    }, // cf. [PDG2008], p. 8
            Parameter::Template{"mass::Z",                                       +91.1855,    +91.1876,    +91.1897   }, // cf. [PDG2008], p. 9
            // Form factor uncertainties
            Parameter::Template{"formfactors::a0_uncertainty",                   +0.85,       +1.0,        +1.15      },
            Parameter::Template{"formfactors::a1_uncertainty",                   +0.85,       +1.0,        +1.15      },
            Parameter::Template{"formfactors::a2_uncertainty",                   +0.85,       +1.0,        +1.15      },
            Parameter::Template{"formfactors::v_uncertainty",                    +0.85,       +1.0,        +1.15      },
            Parameter::Template{"formfactors::xi_perp_uncertainty",              +0.89,       +1.0,        +1.11      },
            Parameter::Template{"formfactors::xi_par_uncertainty",               +0.86,       +1.0,        +1.14      },
            Parameter::Template{"formfactors::fp_uncertainty",                   +0.85,       +1.0,        +1.15      },
            Parameter::Template{"formfactors::f0_uncertainty",                   +0.85,       +1.0,        +1.15      },
            Parameter::Template{"formfactors::ft_uncertainty",                   +0.85,       +1.0,        +1.15      },
            // B LCDA parameters
            Parameter::Template{"f_B",                                           +0.17,       +0.20,       +0.23      }, // GeV, cf. [BHvD2010], Table I
            Parameter::Template{"lambda_B_p",                                    +0.370,      +0.485,      +0.600     }, // GeV, cf. [BHvD2010], Table I
            // B->K^*, K^* LCDA parameters
            Parameter::Template{"B->K^*::a_1_par",                               +0.03,       +0.1,        +0.17      },
            Parameter::Template{"B->K^*::a_2_par",                               +0.0,        +0.1,        +0.2       },
            Parameter::Template{"B->K^*::a_1_perp",                              +0.03,       +0.1,        +0.17      },
            Parameter::Template{"B->K^*::a_2_perp",                              +0.0,        +0.1,        +0.2       },
            Parameter::Template{"B->K^*::f_Kstar_par",                           +0.212,      +0.217,      +0.222     }, // GeV, cf. [BHvD2010], Table I
            Parameter::Template{"B->K^*::f_Kstar_perp@2GeV",                     +0.168,      +0.173,      +0.178     }, // GeV @2 Gev, 0.185 +/-0.005 GeV, cf. [BHvD2010], Table I
            // B->K^*ll uncertainties from subleading terms for Large Recoil
            Parameter::Template{"B->K^*ll::A_0^L_uncertainty@LargeRecoil",       +0.95,       +1.0,        +1.05      },
            Parameter::Template{"B->K^*ll::A_0^R_uncertainty@LargeRecoil",       +0.95,       +1.0,        +1.05      },
            Parameter::Template{"B->K^*ll::A_par^L_uncertainty@LargeRecoil",     +0.95,       +1.0,        +1.05      },
            Parameter::Template{"B->K^*ll::A_par^R_uncertainty@LargeRecoil",     +0.95,       +1.0,        +1.05      },
            Parameter::Template{"B->K^*ll::A_perp^L_uncertainty@LargeRecoil",    +0.95,       +1.0,        +1.05      },
            Parameter::Template{"B->K^*ll::A_perp^R_uncertainty@LargeRecoil",    +0.95,       +1.0,        +1.05      },
            // B->Vll uncertainties at subleading order at Low Recoil
            Parameter::Template{"B->Vll::Lambda_0@LowRecoil",                    -0.5,        +0.0,        +0.5       },
            Parameter::Template{"B->Vll::Lambda_pa@LowRecoil",                   -0.5,        +0.0,        +0.5       },
            Parameter::Template{"B->Vll::Lambda_pp@LowRecoil",                   -0.5,        +0.0,        +0.5       },
            Parameter::Template{"B->Vll::sl_phase_0@LowRecoil",                  -M_PI/2.0,   +0.0,        +M_PI/2.0  },
            Parameter::Template{"B->Vll::sl_phase_pa@LowRecoil",                 -M_PI/2.0,   +0.0,        +M_PI/2.0  },
            Parameter::Template{"B->Vll::sl_phase_pp@LowRecoil",                 -M_PI/2.0,   +0.0,        +M_PI/2.0  },
            // B->X_s HQET parameters
            Parameter::Template{"B->X_s::lambda_1",                              -0.20,       -0.20,       -0.20      }, // cf. [ALGH2001], Table 2, p. 13
            Parameter::Template{"B->X_s::lambda_2",                              +0.12,       +0.12,       +0.12      }, // cf. [ALGH2001], Table 2, p. 13
            // B->X_s gamma SM theory uncertainty
            Parameter::Template{"B->X_sgamma::uncertainty",                      -1.0,        +0.0,        +1.0       },
            // Experimental Input
            Parameter::Template{"exp::BR(B->X_clnu)",                            +0.1042,     +0.1057,     +0.1072    }, // cf. [PDG2008], p. 82
            Parameter::Template{"exp::C(B->X_clnu, B->X_ulnu)",                  +0.57,       +0.58,       +0.59      },
            Parameter::Template{"exp::CKM(B->X_sll, B->X_clnu)",                 +0.975218,   +0.98549,    +0.995277  },
        }));
    }

    Parameter::Parameter(const std::shared_ptr<Parameters::Data> & parameters_data, unsigned index) :
        _parameters_data(parameters_data),
        _index(index)
    {
    }

    Parameter::~Parameter()
    {
    }

    Parameter::operator double () const
    {
        return _parameters_data->data[_index].value;
    }

    double
    Parameter::operator() () const
    {
        return _parameters_data->data[_index].value;
    }

    const Parameter &
    Parameter::operator= (const double & value)
    {
        _parameters_data->data[_index].value = value;

        return *this;
    }

    const double &
    Parameter::central() const
    {
        return _parameters_data->data[_index].central;
    }

    const double &
    Parameter::max() const
    {
        return _parameters_data->data[_index].max;
    }

    const double &
    Parameter::min() const
    {
        return _parameters_data->data[_index].min;
    }

    const std::string &
    Parameter::name() const
    {
        return _parameters_data->data[_index].name;
    }

    UnknownParameterError::UnknownParameterError(const std::string & name) throw () :
        Exception("Unknown parameter: '" + name + "'")
    {
    }
}
