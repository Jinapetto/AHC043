import json
import math
import os
import subprocess

import optuna


# TODO: Write parameter suggestions here
def generate_params(trial: optuna.trial.Trial) -> dict[str, str]:
    # for more information, see https://optuna.readthedocs.io/en/stable/reference/generated/optuna.trial.Trial.html
    params = {
        "yaki_start_income" : str(trial.suggest_int("yaki_start_income", 500, 5000)),
        "yaki_start_money" : str(trial.suggest_int("yaki_start_money", 1000, 10000)),
        "shift_rate_0" : str(trial.suggest_int("shift_rate_0", 1, 5)),
        "shift_rate_1" : str(trial.suggest_int("shift_rate_1", 1, 5)),
        "shift_rate_2" : str(trial.suggest_int("shift_rate_2", 1, 5)),
        "shift_rate_3" : str(trial.suggest_int("shift_rate_3", 1, 5)),
        "beam_width_para" : str(trial.suggest_int("beam_width_para", 50000, 500000)),
        "start_temp_para" : str(trial.suggest_int("start_temp_para", 5000, 100000)),
        "connect_cnt_w_para" : str(trial.suggest_int("connect_cnt_w_para", 50, 4000)),
    }

    return params


# TODO: Customize the score extraction code here
def extract_score(result: dict[str, str]) -> float:
    absolute_score = result["score"]  # noqa: F841
    log10_score = math.log10(absolute_score) if absolute_score > 0.0 else 0.0  # noqa: F841
    relative_score = result["relative_score"]  # noqa: F841

    # score = absolute_score  # for absolute score problems
    score = log10_score       # for relative score problems (alternative)
    # score = relative_score    # for relative score problems

    return score


# TODO: Set the direction to minimize or maximize
def get_direction() -> str:
    # direction = "minimize"
    direction = "maximize"
    return direction


# TODO: Set the timeout (seconds) or the number of trials
def run_optimization(study: optuna.study.Study) -> None:
    study.optimize(Objective(), timeout=86400)
    # study.optimize(Objective(), n_trials=100)


class Objective:
    def __init__(self) -> None:
        pass

    def __call__(self, trial: optuna.trial.Trial) -> float:
        params = generate_params(trial)
        env = os.environ.copy()
        env.update(params)

        scores = []

        cmd = [
            "pahcer",
            "run",
            "--json",
            "--shuffle",
            "--no-result-file",
            "--freeze-best-scores",
        ]

        if trial.number != 0:
            cmd.append("--no-compile")

        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            env=env,
        )

        # see also: https://tech.preferred.jp/ja/blog/wilcoxonpruner/
        for line in process.stdout:
            result = json.loads(line)

            # If an error occurs, stop the process and raise an exception
            if result["error_message"] != "":
                process.send_signal(subprocess.signal.SIGINT)
                raise RuntimeError(result["error_message"])

            score = extract_score(result)
            seed = result["seed"]
            scores.append(score)
            trial.report(score, seed)

            if trial.should_prune():
                print(f"Trial {trial.number} pruned.")
                process.send_signal(subprocess.signal.SIGINT)

                objective_value = sum(scores) / len(scores)
                is_better_than_best = (
                    trial.study.direction == optuna.study.StudyDirection.MINIMIZE
                    and objective_value < trial.study.best_value
                ) or (
                    trial.study.direction == optuna.study.StudyDirection.MAXIMIZE
                    and objective_value > trial.study.best_value
                )

                if is_better_than_best:
                    # Avoid updating the best value
                    raise optuna.TrialPruned()
                else:
                    # It is recommended to return the value of the objective function at the current step
                    # instead of raising TrialPruned.
                    # This is a workaround to report the evaluation information of the pruned Trial to Optuna.
                    return sum(scores) / len(scores)

        return sum(scores) / len(scores)


study = optuna.create_study(
    direction=get_direction(),
    study_name="optuna-study",
    pruner=optuna.pruners.WilcoxonPruner(),
    sampler=optuna.samplers.TPESampler(),
)

run_optimization(study)

print(f"best params = {study.best_params}")
print(f"best score  = {study.best_value}")