#pragma once

#include "correction.h"
#include "corrections.h"

namespace correction {

class TrigCorrProvider : public CorrectionsBase<TrigCorrProvider> {
public:
    enum class UncSource : int {
        Central = -1,
        IsoMu24 = 0,
        singleEle = 1,
        ditau_DM0 = 2,
        ditau_DM1 = 3,
        ditau_3Prong = 4,
    };
    static const std::string& getMuScaleStr(UncScale scale)
    {
        static const std::map<UncScale, std::string> mu_names = {
            { UncScale::Down, "systdown" },
            { UncScale::Central, "nominal" },
            { UncScale::Up, "systup" },
        };
        return mu_names.at(scale);
    }
    static const std::string& getEleScaleStr(UncScale scale)
    {
        static const std::map<UncScale, std::string> ele_names = {
            { UncScale::Down, "sfdown" },
            { UncScale::Central, "sf" },
            { UncScale::Up, "sfup" },
        };
        return ele_names.at(scale);
    }
    static const std::string& getTauScaleStr(UncScale scale)
    {
        static const std::map<UncScale, std::string> tau_names = {
            { UncScale::Down, "down" },
            { UncScale::Central, "nom" },
            { UncScale::Up, "up" },
        };
        return tau_names.at(scale);
    }

    static bool sourceApplies_tau(UncSource source, int Tau_decayMode){
        if(source == UncSource::ditau_DM0 && Tau_decayMode == 0) return true;
        if(source == UncSource::ditau_DM1 && Tau_decayMode == 1 ) return true;
        if(source == UncSource::ditau_3Prong && (Tau_decayMode == 10 || Tau_decayMode == 11) ) return true;
        return false;
    }

    using wpsMapType = std::map<Channel, std::vector<std::pair<std::string, int > > >;
    static bool isTwoProngDM(int dm){
        static const std::set<int> twoProngDMs = { 5,6 };
        return twoProngDMs.count(dm);
    }

    // TrigCorrProvider(const std::string& muon_trg_file, const std::string& ele_trg_file, const std::string& tau_trg_file, const std::string& muon_trg_key, const std::string& ele_trg_key, const std::string& tau_trg_key, const std::string& era) :
    // // TrigCorrProvider(const std::string& muon_trg_file, const std::string& ele_trg_file, const std::string& muon_trg_key, const std::string& ele_trg_key, const std::string& era) :
    //     mutrgcorrections_(CorrectionSet::from_file(muon_trg_file)),
    //     etrgcorrections_(CorrectionSet::from_file(ele_trg_file)),
    //     tautrgcorrections_(CorrectionSet::from_file(tau_trg_file))
    // {
        //     if (era == "2022_Summer22" || era == "2022_Summer22EE" || era == "2023_Summer23" || era == "2023_Summer23BPix"){
            //         muTrgCorrections["Central"]=mutrgcorrections_->at(muon_trg_key);
            //         eleTrgCorrections["Central"]=etrgcorrections_->at(ele_trg_key);
    //         tauTrgCorrections["Central"]=tautrgcorrections_->at(tau_trg_key);
    //     } else {
        //        throw std::runtime_error("Era not supported");
        //     }
        // }
        
        TrigCorrProvider(const std::string& muon_trg_file, const std::string& ele_trg_file, const std::string& tau_trg_file, const std::string& ditauJet_trg_file, const std::string& etau_trg_file, const std::string& mutau_trg_file, const std::string& muon_trg_key, const std::string& ele_trg_key, const std::string& tau_trg_key,  const std::string& era) :
        // TrigCorrProvider(const std::string& muon_trg_file, const std::string& ele_trg_file, const std::string& muon_trg_key, const std::string& ele_trg_key, const std::string& era) :
        mutrgcorrections_(CorrectionSet::from_file(muon_trg_file)),
        etrgcorrections_(CorrectionSet::from_file(ele_trg_file)),
        tautrgcorrections_(CorrectionSet::from_file(tau_trg_file)),
        ditauJet_trgcorrections_(CorrectionSet::from_file(ditauJet_trg_file)),
        etau_trgcorrections_(CorrectionSet::from_file(etau_trg_file)),
        mutau_trgcorrections_(CorrectionSet::from_file(mutau_trg_file))
        {
            if (era == "2022_Summer22"){// || era == "2022_Summer22EE" || era == "2023_Summer23" || era == "2023_Summer23BPix"){
                // muTrgCorrections["Central"]=mutrgcorrections_->at(muon_trg_key);
            muTrgCorrections["Central"]=mutrgcorrections_->at(muon_trg_key);
            eleTrgCorrections["Central"]=etrgcorrections_->at(ele_trg_key);
            tauTrgCorrections["Central"]=tautrgcorrections_->at(tau_trg_key);
            eleTrgCorrections_Mc["Central"]=etrgcorrections_->at("Electron-HLT-McEff");
            eleTrgCorrections_Data["Central"]=etrgcorrections_->at("Electron-HLT-DataEff");
            tauTrgCorrections["Central"]=tautrgcorrections_->at("tau_trigger");
            ditauJet_trgCorrections["Central"]=ditauJet_trgcorrections_->at("jetlegSFs");
            etau_trgCorrections_Mc["Central"]=etau_trgcorrections_->at("Electron-HLT-McEff");
            etau_trgCorrections_Data["Central"]=etau_trgcorrections_->at("Electron-HLT-DataEff");
            mutau_trgCorrections_Mc["Central"]=mutau_trgcorrections_->at("NUM_IsoMu20_DEN_CutBasedIdTight_and_PFIsoTight_MCeff");
            mutau_trgCorrections_Data["Central"]=mutau_trgcorrections_->at("NUM_IsoMu20_DEN_CutBasedIdTight_and_PFIsoTight_DATAeff");
        } else {
           throw std::runtime_error("Era not supported");
        }
    }

    float getEff_singleEle(const LorentzVectorM & part_p4, std::string year, UncSource source, UncScale scale) const {
        float eff = 1;
        const std::string& scale_str = getEleScaleStr(scale);
        std::string Working_Point = "HLT_SF_Ele30_TightID";
        eff = eleTrgCorrections_Mc.at("Central")->evaluate({year, "nom", Working_Point, part_p4.Eta(), part_p4.Pt()});
        return eff ;
    }

    float getSF_singleEleWpTight(const LorentzVectorM & part_p4, std::string year, UncSource source, UncScale scale) const {
        float corr_SF = 1;
        const std::string& scale_str = getEleScaleStr(scale);
        std::string Working_Point = "HLT_SF_Ele30_MVAiso80ID";
        corr_SF = eleTrgCorrections.at(getUncSourceName(source))->evaluate({year, scale_str, Working_Point, part_p4.Eta(), part_p4.Pt()});
        return corr_SF ;
    }
    float getSF_singleIsoMu(const LorentzVectorM & part_p4, std::string year, UncSource source, UncScale scale) const {
        float corr_SF = 1;
        const std::string& scale_str = getMuScaleStr(scale);
        corr_SF = muTrgCorrections.at(getUncSourceName(source))->evaluate({ abs(part_p4.Eta()), part_p4.Pt(), scale_str});
        return corr_SF ;
    }
    float getSF_ditau(const LorentzVectorM & part_p4, std::string year, int Tau_decayMode, std::string trigtype, std::string wp, std::string corrtype, UncSource source, UncScale scale) const {
        float corr_SF = 1;
        const UncScale tau_scale = sourceApplies_tau(source, Tau_decayMode) ? scale : UncScale::Central;
        const std::string& scale_str = getTauScaleStr(tau_scale);
        corr_SF = tauTrgCorrections.at(getUncSourceName(source))->evaluate({ part_p4.Pt(), Tau_decayMode, trigtype, wp, corrtype, scale_str });
        return corr_SF ;
    }
private:
    static std::string& getUncSourceName(UncSource source) {
        static std::string sourcename = "Central";
        if (source==UncSource::IsoMu24) sourcename = "Central"; //Still get from the central Key, maybe rename later
        if (source==UncSource::singleEle) sourcename = "Central"; //Still get from the central Key, maybe rename later
        if (source==UncSource::ditau_DM0) sourcename = "Central"; //Still get from the central Key, maybe rename later
        if (source==UncSource::ditau_DM1) sourcename = "Central"; //Still get from the central Key, maybe rename later
        if (source==UncSource::ditau_3Prong) sourcename = "Central"; //Still get from the central Key, maybe rename later

        //(if source==SourceName) sourcename = SourceName
        return sourcename;
    }

private:
    std::unique_ptr<CorrectionSet> mutrgcorrections_, etrgcorrections_, tautrgcorrections_, ditauJet_trgcorrections_, etau_trgcorrections_, mutau_trgcorrections_;
    std::map<std::string, Correction::Ref> muTrgCorrections, eleTrgCorrections, eleTrgCorrections_Mc, eleTrgCorrections_Data, tauTrgCorrections, ditauJet_trgCorrections, etau_trgCorrections_Mc, etau_trgCorrections_Data, mutau_trgCorrections_Mc, mutau_trgCorrections_Data;
    const std::string period_;
} ;



} // namespace correction